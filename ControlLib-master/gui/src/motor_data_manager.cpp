#include "motor_data_manager.hpp"

#include "dji_motor.hpp"
#include "io.hpp"

#include <chrono>
#include <cmath>
#include <exception>
#include <stdexcept>
#include <string>

MotorDataManager::MotorDataManager(QObject *parent)
    : QObject(parent) {
}

MotorDataManager::~MotorDataManager() {
    stop();
    // DJIMotorManager uses a detached process-wide send loop and has no unregister API.
    // Keep the registered motor alive until process exit so the manager never sees a dangling pointer.
    if (motor_registered_) {
        motor_.release();
    }
}

bool MotorDataManager::start(const QString &canName, int motorId) {
    if (running_.load()) {
        return true;
    }

    if (!ensureMotor(canName, motorId)) {
        return false;
    }

    running_ = true;
    collection_thread_ = std::thread(&MotorDataManager::dataCollectionThread, this);
    return true;
}

void MotorDataManager::stop() {
    const bool was_running = running_.exchange(false);
    if (was_running && collection_thread_.joinable()) {
        collection_thread_.join();
    }

    if (motor_) {
        motor_->set_directly(0.0f);
    }
}

bool MotorDataManager::isRunning() const {
    return running_.load();
}

bool MotorDataManager::isMotorReady() const {
    return motor_registered_ && motor_ != nullptr;
}

void MotorDataManager::applyControlParameters(float targetSpeed, float kp, float ki, float kd) {
    std::lock_guard<std::mutex> lock(control_mutex_);
    params_.target_speed = targetSpeed;
    params_.kp = kp;
    params_.ki = ki;
    params_.kd = kd;
    configurePidLocked();
    resetPidStateLocked();
}

MotorDataManager::ControlParameters MotorDataManager::controlParameters() const {
    std::lock_guard<std::mutex> lock(control_mutex_);
    return params_;
}

bool MotorDataManager::ensureMotor(const QString &canName, int motorId) {
    const QString normalized_can = canName.trimmed();
    if (normalized_can.isEmpty()) {
        emit error(QStringLiteral("CAN 设备名不能为空"));
        return false;
    }

    if (motorId < 1 || motorId > 8) {
        emit error(QStringLiteral("GM6020 电机 ID 必须在 1 到 8 之间"));
        return false;
    }

    if (motor_registered_) {
        if (can_name_ == normalized_can && motor_id_ == motorId) {
            return true;
        }

        emit error(QStringLiteral("当前进程已注册一个 GM6020 电机；如需更换 CAN 或 ID，请重启程序"));
        return false;
    }

    const std::string can_name = normalized_can.toStdString();

    try {
        if (!can_registered_) {
            try {
                IO::io<CAN>.insert(can_name);
            } catch (const std::runtime_error &e) {
                const std::string message = e.what();
                if (message.find("double register device") == std::string::npos) {
                    throw;
                }
            }
            can_registered_ = true;
        }

        Hardware::DJIMotorManager::start();
        Hardware::DJIMotorConfig config(Hardware::DJIMotorConfig::TypeCast(6020), can_name, motorId);
        motor_ = std::make_unique<Hardware::DJIMotor>(config);
        motor_->enable();
    } catch (const std::exception &e) {
        emit error(QStringLiteral("初始化 CAN/GM6020 失败：%1").arg(QString::fromLocal8Bit(e.what())));
        motor_.reset();
        return false;
    }

    if (!motor_ || !motor_->motor_enabled_) {
        emit error(QStringLiteral("GM6020 注册失败，请检查 CAN 设备和电机 ID"));
        motor_.reset();
        return false;
    }

    can_name_ = normalized_can;
    motor_id_ = motorId;
    motor_registered_ = true;

    {
        std::lock_guard<std::mutex> lock(control_mutex_);
        configurePidLocked();
        resetPidStateLocked();
    }
    return true;
}

void MotorDataManager::dataCollectionThread() {
    const auto start_time = std::chrono::steady_clock::now();
    auto next_sample_time = start_time;

    while (running_.load()) {
        const auto now = std::chrono::steady_clock::now();
        const double elapsed = std::chrono::duration<double>(now - start_time).count();

        ControlParameters params;
        int16_t command_current = 0;
        {
            std::lock_guard<std::mutex> lock(control_mutex_);
            params = params_;
            command_current = updateCurrentCommand(params);
            last_command_current_ = command_current;
        }

        if (now >= next_sample_time) {
            monitor_.addData(makeSample(elapsed, params, command_current));
            emit dataReady();
            next_sample_time = now + std::chrono::milliseconds(kSampleIntervalMs);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(kControlIntervalMs));
    }
}

SpeedData MotorDataManager::makeSample(double timestamp,
                                       const ControlParameters &params,
                                       int16_t commandCurrent) const {
    SpeedData data;
    data.timestamp = timestamp;
    data.motor_id = motor_id_;
    data.target_speed = params.target_speed;
    data.kp = params.kp;
    data.ki = params.ki;
    data.kd = params.kd;

    if (!motor_) {
        return data;
    }

    data.actual_speed = motor_->data_.rotor_angular_velocity;
    data.error = data.target_speed - data.actual_speed;
    data.actual_current = static_cast<double>(motor_->motor_measure_.given_current) * kRawCurrentToAmp;
    data.raw_current = motor_->motor_measure_.given_current;
    data.command_current = commandCurrent;
    data.rotor_angle = motor_->data_.rotor_angle;
    data.encoder = motor_->motor_measure_.ecd;
    data.speed_rpm = motor_->motor_measure_.speed_rpm;
    data.temperature = motor_->motor_measure_.temperate;
    return data;
}

int16_t MotorDataManager::updateCurrentCommand(const ControlParameters &params) {
    if (!motor_ || !speed_pid_) {
        return 0;
    }

    speed_pid_->set(params.target_speed);

    const auto command = static_cast<int16_t>(std::lround(speed_pid_->out));
    motor_->set_directly(static_cast<float>(command));
    return command;
}

void MotorDataManager::configurePidLocked() {
    if (!motor_) {
        speed_pid_.reset();
        return;
    }

    const Pid::PidConfig config{
        params_.kp,
        params_.ki,
        params_.kd,
        kMaxCurrentCommand,
        kMaxIntegralOutput,
    };

    if (!speed_pid_) {
        speed_pid_ = std::make_unique<Pid::PidPosition>(config, motor_->data_.rotor_angular_velocity);
        return;
    }

    speed_pid_->kp = config.kp;
    speed_pid_->ki = config.ki;
    speed_pid_->kd = config.kd;
    speed_pid_->max_out = config.max_out;
    speed_pid_->max_iout = config.max_iout;
}

void MotorDataManager::resetPidStateLocked() {
    if (speed_pid_) {
        speed_pid_->clean();
    }
    last_command_current_ = 0;
}

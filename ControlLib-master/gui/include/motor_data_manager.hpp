#pragma once

#include <QObject>
#include <QString>
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

#include "pid.hpp"
#include "speed_monitor.hpp"

namespace Hardware {
    class DJIMotor;
}

class MotorDataManager : public QObject {
    Q_OBJECT

public:
    struct ControlParameters {
        float target_speed = 0.0f;
        float kp = 0.0f;
        float ki = 0.0f;
        float kd = 0.0f;
    };

    explicit MotorDataManager(QObject *parent = nullptr);
    ~MotorDataManager() override;

    bool start(const QString &canName, int motorId);
    void stop();
    bool isRunning() const;
    bool isMotorReady() const;

    void applyControlParameters(float targetSpeed, float kp, float ki, float kd);
    ControlParameters controlParameters() const;

    const SpeedMonitor *getMonitor() const { return &monitor_; }
    SpeedMonitor *getMonitor() { return &monitor_; }

signals:
    void dataReady();
    void error(const QString &message);

private:
    bool ensureMotor(const QString &canName, int motorId);
    void dataCollectionThread();
    SpeedData makeSample(double timestamp, const ControlParameters &params, int16_t commandCurrent) const;
    int16_t updateCurrentCommand(const ControlParameters &params);
    void configurePidLocked();
    void resetPidStateLocked();

    static constexpr int kControlIntervalMs = 10;
    static constexpr int kSampleIntervalMs = 50;
    static constexpr float kMaxCurrentCommand = 30000.0f;
    static constexpr float kMaxIntegralOutput = 15000.0f;
    static constexpr double kRawCurrentToAmp = 20.0 / 16384.0;

    SpeedMonitor monitor_;

    std::unique_ptr<Hardware::DJIMotor> motor_;
    std::unique_ptr<Pid::PidPosition> speed_pid_;
    QString can_name_;
    int motor_id_ = 0;
    bool can_registered_ = false;
    bool motor_registered_ = false;

    std::thread collection_thread_;
    std::atomic<bool> running_{false};

    mutable std::mutex control_mutex_;
    ControlParameters params_;
    int16_t last_command_current_ = 0;
};

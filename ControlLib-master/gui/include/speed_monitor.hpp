#pragma once

#include <QObject>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <vector>

struct SpeedData {
    double timestamp = 0.0;
    int motor_id = 0;

    double target_speed = 0.0;
    double actual_speed = 0.0;
    double error = 0.0;

    double actual_current = 0.0;
    int16_t raw_current = 0;
    int16_t command_current = 0;

    double rotor_angle = 0.0;
    uint16_t encoder = 0;
    int16_t speed_rpm = 0;
    uint8_t temperature = 0;

    double kp = 0.0;
    double ki = 0.0;
    double kd = 0.0;
};

class SpeedMonitor : public QObject {
    Q_OBJECT

public:
    explicit SpeedMonitor(QObject *parent = nullptr);

    void addData(const SpeedData &data);
    std::vector<SpeedData> getDataSnapshot() const;
    bool getLatestData(SpeedData &data) const;
    size_t size() const;

    void clear();

    double getTargetSpeedAvg() const;
    double getActualSpeedAvg() const;
    double getMaxError() const;
    double getMinError() const;

signals:
    void dataUpdated();
    void dataCleared();

private:
    std::vector<SpeedData> data_;
    double start_time_ = 0.0;
    mutable std::mutex data_mutex_;
};

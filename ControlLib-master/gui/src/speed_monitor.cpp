#include "speed_monitor.hpp"

#include <algorithm>
#include <cmath>

SpeedMonitor::SpeedMonitor(QObject *parent)
    : QObject(parent), start_time_(0.0) {
}

void SpeedMonitor::addData(const SpeedData &data) {
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        if (data_.empty()) {
            start_time_ = data.timestamp;
        }
        data_.push_back(data);
    }

    emit dataUpdated();
}

std::vector<SpeedData> SpeedMonitor::getDataSnapshot() const {
    std::lock_guard<std::mutex> lock(data_mutex_);
    return data_;
}

bool SpeedMonitor::getLatestData(SpeedData &data) const {
    std::lock_guard<std::mutex> lock(data_mutex_);
    if (data_.empty()) {
        return false;
    }

    data = data_.back();
    return true;
}

size_t SpeedMonitor::size() const {
    std::lock_guard<std::mutex> lock(data_mutex_);
    return data_.size();
}

void SpeedMonitor::clear() {
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        data_.clear();
        start_time_ = 0.0;
    }

    emit dataCleared();
}

double SpeedMonitor::getTargetSpeedAvg() const {
    std::lock_guard<std::mutex> lock(data_mutex_);
    if (data_.empty()) {
        return 0.0;
    }

    double sum = 0.0;
    for (const auto &d : data_) {
        sum += d.target_speed;
    }
    return sum / static_cast<double>(data_.size());
}

double SpeedMonitor::getActualSpeedAvg() const {
    std::lock_guard<std::mutex> lock(data_mutex_);
    if (data_.empty()) {
        return 0.0;
    }

    double sum = 0.0;
    for (const auto &d : data_) {
        sum += d.actual_speed;
    }
    return sum / static_cast<double>(data_.size());
}

double SpeedMonitor::getMaxError() const {
    std::lock_guard<std::mutex> lock(data_mutex_);
    if (data_.empty()) {
        return 0.0;
    }

    return std::max_element(data_.begin(), data_.end(),
        [](const SpeedData &a, const SpeedData &b) {
            return std::abs(a.error) < std::abs(b.error);
        })->error;
}

double SpeedMonitor::getMinError() const {
    std::lock_guard<std::mutex> lock(data_mutex_);
    if (data_.empty()) {
        return 0.0;
    }

    return std::min_element(data_.begin(), data_.end(),
        [](const SpeedData &a, const SpeedData &b) {
            return std::abs(a.error) < std::abs(b.error);
        })->error;
}

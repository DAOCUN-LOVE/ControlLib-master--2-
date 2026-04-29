# 实现摘要

本 GUI 已按 GM6020 单电机控制需求更新：

- `MainWindow`：提供 CAN/ID 选择、目标速度和 PID 输入、“应用参数”、开始/停止、暂停绘图、清除和 CSV 保存。
- `MotorDataManager`：负责初始化 SocketCAN 与 GM6020，运行 10 ms 控制循环和 50 ms 采样循环，记录速度、电流、原始反馈和 PID 参数。
- `SpeedMonitor`：线程安全保存历史数据，供曲线、日志和 CSV 导出读取。
- `SimpleChart`：支持双 Y 轴、多曲线、实线/虚线、显隐控制、拖拽和平移缩放。
- `dji_motor.cpp`：GM6020 ID 范围扩展为 1-8。

CSV 字段：

```csv
time_s,motor_id,target_speed_rad_s,actual_speed_rad_s,error_rad_s,actual_current_a,raw_current,command_current,encoder,speed_rpm,temperature_c,kp,ki,kd
```

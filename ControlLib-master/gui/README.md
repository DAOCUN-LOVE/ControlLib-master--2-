# GM6020 单电机 PID 控制与数据监视 GUI

基于 Qt5/C++ 的 Ubuntu GUI，用于通过 SocketCAN 控制一个 GM6020 电机，并实时查看速度、电流、原始 motordata 和历史 CSV。

## 功能

- GM6020 电机 ID 1-8 可选，单次运行只控制一个电机。
- Kp、Ki、Kd 和目标速度在输入框中编辑，点击“应用参数”后一次性下发到控制线程。
- 双 Y 轴实时曲线：
  - 左 Y 轴：速度 rad/s，实际速度为蓝色实线，目标速度为红色虚线并可勾选显示。
  - 右 Y 轴：电流 A，实际电流为绿色实线并可勾选显示。
- 只读日志框逐行打印 motordata：时间、ID、目标速度、实际速度、电流、原始电流、命令电流、编码器、RPM、温度和 PID 参数。
- “暂停绘图”只暂停曲线刷新，不停止数据记录和日志追加。
- 支持清除图表/日志/历史数据，支持保存历史 CSV。

## 构建

```bash
sudo apt install build-essential cmake qtbase5-dev
cd gui
cmake -S . -B build
cmake --build build -j
```

## 运行

先配置 SocketCAN，例如：

```bash
sudo ip link set can0 up type can bitrate 1000000
```

然后运行：

```bash
./build/SpeedCurveGUI
```

在界面中选择 CAN 设备和 GM6020 ID，设置目标速度与 PID 参数，点击“开始控制”。如果要更换 CAN 设备或电机 ID，请在开始控制前选择；成功注册电机后，本进程会保持只控制这一台电机。

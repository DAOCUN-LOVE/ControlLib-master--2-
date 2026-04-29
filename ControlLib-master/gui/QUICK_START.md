# 快速开始

## 1. 安装依赖

```bash
sudo apt install build-essential cmake qtbase5-dev
```

## 2. 编译

```bash
cd gui
cmake -S . -B build
cmake --build build -j
```

## 3. 准备 CAN

根据你的设备配置 `can0`，示例：

```bash
sudo ip link set can0 up type can bitrate 1000000
```

## 4. 启动 GUI

```bash
./build/SpeedCurveGUI
```

## 5. 使用流程

1. 选择 CAN 设备和 GM6020 ID，ID 范围为 1-8。
2. 输入目标速度、Kp、Ki、Kd。
3. 点击“应用参数”，一次性下发目标速度和 PID 参数。
4. 点击“开始控制”开始闭环控制与数据记录。
5. 根据需要勾选目标速度曲线或电流曲线。
6. 点击“暂停绘图”只停止曲线刷新，数据仍会继续记录并追加到日志。
7. 点击“保存历史 CSV”导出完整历史数据。

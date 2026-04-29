# 🎯 电机转速曲线实时显示 GUI - 完整使用指南

## ✅ 项目已完成！

已为你的 ControlLib 项目创建了一个功能完整的 Qt5 GUI 应用，用于实时显示电机的目标转速和实际转速曲线。

## 📦 已生成的文件

```
gui/
├── include/              # 头文件目录
│   ├── main_window.hpp          ✓ 主窗口
│   ├── speed_monitor.hpp        ✓ 转速监测
│   ├── motor_data_manager.hpp   ✓ 电机数据管理
│   └── simple_chart.hpp         ✓ 图表控件
├── src/                  # 源文件目录
│   ├── main.cpp                 ✓ 程序入口
│   ├── main_window.cpp          ✓ 主窗口实现
│   ├── speed_monitor.cpp        ✓ 转速监测实现
│   ├── motor_data_manager.cpp   ✓ 数据管理实现
│   └── simple_chart.cpp         ✓ 图表实现
├── build/               # 编译输出
│   └── SpeedCurveGUI    ✓ 可执行文件（已编译）
├── CMakeLists.txt              ✓ 编译配置
├── build.sh                    ✓ 编译脚本
├── README.md                   ✓ 详细文档
├── QUICK_START.md              ✓ 快速开始
├── SUMMARY.md                  ✓ 项目总结
└── example_integration.cpp     ✓ 集成示例
```

## 🚀 快速运行

直接运行已编译的可执行文件：

```bash
/home/daocunbaoyue/桌面/ControlLib/gui/build/SpeedCurveGUI
```

## 🔨 重新编译（如需修改代码）

```bash
cd /home/daocunbaoyue/桌面/ControlLib/gui
chmod +x build.sh
./build.sh
```

或手动编译：

```bash
cd /home/daocunbaoyue/桌面/ControlLib/gui
mkdir -p build
cd build
cmake ..
make -j4
./SpeedCurveGUI
```

## 💡 主要功能

### 1. 实时曲线显示
- **蓝色曲线**：目标转速 (rad/s)
- **红色曲线**：实际转速 (rad/s)
- 自动缩放和网格显示

### 2. 参数控制面板
- **目标转速**：设置 0-1000 rad/s
- **Kp 参数**：0.0 - 100.0
- **Ki 参数**：0.0 - 100.0
- **Kd 参数**：0.0 - 100.0

### 3. 数据管理
- 📊 **实时显示**：数据点数、当前转速、误差值
- 💾 **保存数据**：导出为 CSV 格式
- 📸 **导出图表**：保存曲线为 PNG 图片
- 🗑️ **清除数据**：重新开始监测

### 4. 交互操作
- **鼠标拖拽**：平移图表视图
- **鼠标滚轮**：缩放图表
- **动态调整**：实时修改参数观看效果

## 📊 使用流程

```
启动应用
    ↓
设置目标转速 (如：100 rad/s)
    ↓
调整 PID 参数 (Kp=1.0, Ki=0.1, Kd=0.01)
    ↓
点击 "Start Monitoring"
    ↓
观察实时曲线
    ↓
点击 "Save Data (CSV)" 保存
    ↓
点击 "Export Image (PNG)" 导出图表
    ↓
点击 "Stop Monitoring" 停止
```

## 🔌 与电机控制库集成

### 方法1：使用 example_integration.cpp

```cpp
// 1. 初始化电机
Hardware::DJIMotor motor(6020, "can0", 1);
motor.enable();

// 2. 创建PID控制器
Pid::PidPosition pid(M6020_SPEED_PID_CONFIG, motor.data_.output_angular_velocity);
motor.setCtrl(pid);

// 3. 启动GUI（自动采集电机数据）
// MotorDataManager 会从 motor.data_ 读取实时数据
```

### 方法2：修改现有 demo2.cpp

在 `demo2.cpp` 中替换 main 函数为 GUI 版本：

```cpp
#include "gui/include/main_window.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // ... 电机初始化代码 ...
    Hardware::DJIMotor motor(6020, "can0", 1);
    
    // 启动GUI
    MainWindow window;
    window.show();
    
    return app.exec();
}
```

## 📈 CSV 导出格式

保存的数据格式如下：

```csv
Time(s),TargetSpeed(rad/s),ActualSpeed(rad/s),Error(rad/s)
0.050,100.00,95.20,4.80
0.100,100.00,97.50,2.50
0.150,100.00,99.10,0.90
...
```

可用于：
- 📊 Excel 或 Matlab 进行数据分析
- 📉 绘制更复杂的图表
- 📐 计算系统特性参数

## ⚙️ 自定义配置

### 修改采样频率

编辑 `gui/src/motor_data_manager.cpp` 第 60 行：

```cpp
// 改变延迟时间（单位：ms）
UserLib::sleep_ms(50);  // 默认 20Hz
// UserLib::sleep_ms(100);  // 改为 10Hz
// UserLib::sleep_ms(20);   // 改为 50Hz
```

### 修改图表颜色

编辑 `gui/src/simple_chart.cpp` 中的 `setPenColor()` 方法

### 修改窗口大小

编辑 `gui/src/main_window.cpp` 第 9 行：

```cpp
setGeometry(100, 100, 1200, 800);  // x, y, 宽度, 高度
```

## 🐛 常见问题

### Q: 运行时出现"找不到Qt库"错误

**A:** 安装 Qt5 运行库：
```bash
sudo apt-get install libqt5gui5 libqt5core5a
```

### Q: 如何修改图表刷新速度

**A:** 在 `main_window.cpp` 中修改定时器间隔：
```cpp
update_timer_->start(100);  // 100ms 更新一次
```

### Q: CSV 中没有数据

**A:** 确保已点击 "Start Monitoring" 至少收集了一些数据后再保存

## 📚 相关文档

- **README.md** - 详细功能说明和使用指南
- **QUICK_START.md** - 编译和集成步骤
- **SUMMARY.md** - 项目技术总结
- **example_integration.cpp** - 完整集成示例代码

## 🎨 项目架构

```
Qt5 GUI (SpeedCurveGUI)
    ├── MainWindow (主窗口)
    │   ├── SimpleChart (图表控件)
    │   ├── 参数控制面板
    │   └── 状态显示面板
    ├── MotorDataManager (数据管理)
    │   ├── 数据采集线程
    │   ├── 线程安全通信
    │   └── SpeedMonitor (转速监测)
    └── 与 ControlLib 的接口
        └── Hardware::DJIMotor (电机对象)
```

## 📊 系统要求

- **操作系统**：Linux (Ubuntu/Debian) / Windows / macOS
- **Qt 版本**：5.12+ （已验证：5.15.13）
- **编译器**：GCC 5.0+ 或 Clang 3.8+
- **CMake**：3.20+

## ✨ 项目特色

✅ **零外部依赖** - 不需要 QCustomPlot 等额外库  
✅ **即插即用** - 预编译可执行文件  
✅ **完全开源** - 可自由修改和扩展  
✅ **高性能** - 内存占用少，响应快速  
✅ **易集成** - 无缝对接现有控制库  

## 🚀 下一步

1. **运行应用**: `./gui/build/SpeedCurveGUI`
2. **集成电机**: 按需修改代码传入电机对象
3. **测试功能**: 观察实时曲线显示效果
4. **导出数据**: 保存 CSV 进行数据分析

---

**祝你使用愉快！** 🎉

如有问题，请查看详细文档或修改源代码进行自定义。

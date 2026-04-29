/**
 * GUI 集成使用示例
 *
 * 这个文件展示了如何将 SpeedCurveGUI 与现有的电机控制库集成
 */

#include <QApplication>
#include "gui/include/main_window.hpp"
#include "include/dji_motor.hpp"
#include "include/io.hpp"
#include "include/pid.hpp"
#include "include/config.hpp"
#include <thread>

// 电机控制线程
void motorControlThread(Hardware::DJIMotor *motor) {
    while (true) {
        motor->set(1);  // 设置电流输出
        UserLib::sleep_ms(1);
    }
}

int main(int argc, char *argv[]) {
    // 初始化 CAN 接口
    IO::io<CAN>.insert("can0");
    Hardware::DJIMotorManager::start();

    // 创建电机对象
    Hardware::DJIMotor motor(6020, "can0", 1);
    motor.enable();

    // 创建 PID 控制器
    Pid::PidPosition pid(M6020_SPEED_PID_CONFIG, motor.data_.output_angular_velocity);
    motor.setCtrl(pid);

    // 在后台启动电机控制线程
    std::thread control_thread(motorControlThread, &motor);

    // 启动 GUI 应用
    QApplication app(argc, argv);

    MainWindow window;

    // 将电机对象传给 GUI（需要修改 MainWindow 构造函数接收电机对象）
    // window.setMotor(&motor);

    window.show();

    int result = app.exec();

    // 清理
    if (control_thread.joinable()) {
        control_thread.join();
    }

    return result;
}

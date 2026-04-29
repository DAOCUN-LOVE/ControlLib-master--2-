#include "dji_motor.hpp"
#include "io.hpp"
#include "pid.hpp"
#include "config.hpp"

int main() {
    IO::io<CAN>.insert("can0");
    Hardware::DJIMotorManager::start();


    Hardware::DJIMotor motor(6020, "can0", 1);
    motor.enable();

    Pid::PidPosition pid(M6020_SPEED_PID_CONFIG, motor.data_.output_angular_velocity);
    motor.setCtrl(pid);
    
    while (true) {
        motor.set(1);
        printf("angle: %f, velocity: %f\n", motor.data_.rotor_angle, motor.data_.rotor_angular_velocity);
        UserLib::sleep_ms(1);
    }
}
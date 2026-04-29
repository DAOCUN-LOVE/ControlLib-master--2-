#include "can.hpp"
#include "user_lib.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

namespace IO
{
    Can_interface::Can_interface(const std::string& name) : name(name) {
        addr = new sockaddr_can;
        ifr = new ifreq;
        soket_id = -1;
        init_flag = false;
        init(name.c_str());
    }

    void Can_interface::init(const char* can_channel) {
        // create CAN socket
        if ((soket_id = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
            throw std::runtime_error(std::string("Error while creating CAN socket: ") + std::strerror(errno));
        }

        std::strcpy(ifr->ifr_name, can_channel);
        if (ioctl(soket_id, SIOCGIFINDEX, ifr) < 0) {
            const std::string message = std::string("Error getting CAN interface index for ") +
                                        can_channel + ": " + std::strerror(errno);
            close(soket_id);
            soket_id = -1;
            throw std::runtime_error(message);
        }

        addr->can_family = AF_CAN;
        addr->can_ifindex = ifr->ifr_ifindex;

        // bind CAN socket to the given interface
        if (bind(soket_id, (sockaddr *)addr, sizeof(*addr)) < 0) {
            const std::string message = std::string("Error binding CAN socket to ") +
                                        can_channel + ": " + std::strerror(errno);
            close(soket_id);
            soket_id = -1;
            throw std::runtime_error(message);
        }
        init_flag = true;
    }

    Can_interface::~Can_interface() {
        if (soket_id >= 0) {
            close(soket_id);
        }
        delete addr;
        delete ifr;
    }

    bool Can_interface::task() {
        for (;;) {
            if (init_flag) {
                // read CAN frame
                if (read(soket_id, &frame_r, sizeof(can_frame)) <= 0) {
                    LOG_ERR("Error reading CAN frame");
                    return false;
                } else {
                    // printf("not reading!\n");
                }
                callback_key(frame_r.can_id, frame_r);
            }
        }
    }

    bool Can_interface::send(const can_frame &frame) {
        /* send CAN frame */
        return write(soket_id, &frame, sizeof(can_frame)) == sizeof(can_frame);
    }

}  // namespace Hardware

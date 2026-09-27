#ifndef HARDWAREDEVICES_H
#define HARDWAREDEVICES_H

#include <string>
#include <vector>

struct HardwareDeviceInfo {
    int index = 0;
    std::string id;
    std::string name;
};

std::vector<HardwareDeviceInfo> enumerateDirectMLDevices();
int findDirectMLDeviceIndex(const std::string &deviceId);

#endif

#ifndef DISPLAY_DEVICES_H
#define DISPLAY_DEVICES_H

#include <string>
#include <vector>

struct DisplayDescriptor {
    std::string id;
    std::string name;
    std::string deviceName;

    int desktopX = 0;
    int desktopY = 0;
    int desktopWidth = 0;
    int desktopHeight = 0;

    int adapterIndex = -1;
    int outputIndex = -1;

    int captureWidth = 0;
    int captureHeight = 0;
};

std::vector<DisplayDescriptor> enumerateDisplays();
bool resolveDisplay(const std::string &displayId, DisplayDescriptor &descriptor);

#endif

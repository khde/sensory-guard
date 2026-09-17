#ifndef ENGINECONFIG_H
#define ENGINECONFIG_H

#include <unordered_set>
#include <string>

struct EngineConfig {
    float confidenceThreshold = 0.20f;
    std::unordered_set<std::string> enabledLabels;
    int maxFps = 20;
};

#endif
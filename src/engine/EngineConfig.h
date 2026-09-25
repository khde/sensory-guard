#ifndef ENGINECONFIG_H
#define ENGINECONFIG_H

#include <unordered_set>
#include <string>

struct EngineConfig {
    float confidenceThreshold = 0.20f;
    bool ignoreSmallScreenChanges = true;
    float frameChangeThreshold = 0.015f;
    std::unordered_set<std::string> enabledLabels;
    int maxFps = 12;
};

#endif
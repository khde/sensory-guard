#ifndef CENSORINGCONFIG_H
#define CENSORINGCONFIG_H

enum class CensoringStyle {
    Black,
    GaussianBlur,
    Pixelation
};

struct CensoringConfig {
    CensoringStyle style = CensoringStyle::Black;
    int blurIntensity = 30;
    int pixelSize = 16;
    float scaleFactor = 1.0f;
};

#endif

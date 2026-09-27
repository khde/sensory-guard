#ifndef ONNX_DETECTOR_H
#define ONNX_DETECTOR_H

#include "inference/Detection.h"
#include "engine/EngineConfig.h"

#include <string>
#include <vector>
#include <memory>
#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>

class OnnxDetector {
public:
    enum class InitializationError {
        None,
        InvalidConfiguration,
        BackendUnavailable,
        DeviceUnavailable,
        ModelLoadFailed
    };

    OnnxDetector(const std::string& modelPath, const HardwareConfig &hardwareConfig, InitializationError &errorCode, std::string &errorMessage);
    ~OnnxDetector();

    bool isReady() const;

    std::vector<float> inference(const std::vector<float>& inputTensor);

    std::vector<DetectionResult> detect(const cv::Mat &image, float confidenceThreshold);
    
private:
    Ort::Env m_env;
    Ort::SessionOptions m_sessionOptions;
    std::unique_ptr<Ort::Session> m_session;
    bool m_ready = false;

    // Input/Output names
    const char* m_inputName = "images";
    const char* m_outputName = "output0";

    // Shape
    const std::vector<int64_t> m_inputShape = {1, 3, 640, 640}; 
};

#endif
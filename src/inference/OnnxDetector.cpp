#include "OnnxDetector.h"

#include <iostream>

OnnxDetector::OnnxDetector(const std::string& modelPath) {
    m_env = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "SensorGuardInference");
    m_sessionOptions.SetIntraOpNumThreads(4);
    m_sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

    loadModel(modelPath);
}

OnnxDetector::~OnnxDetector() {
}

bool OnnxDetector::loadModel(const std::string& modelPath) {
    try {
        m_session = std::make_unique<Ort::Session>(m_env, modelPath.c_str(), m_sessionOptions);
        std::cout << "Successfully loaded ONNX model from: " << modelPath << std::endl;
        return true;
    } catch (const Ort::Exception& e) {
        std::cerr << "Failed to load ONNX model: " << e.what() << std::endl;
        return false;
    }
}

std::vector<float> OnnxDetector::inference(const std::vector<float>& inputTensor) {
    return std::vector<float>();
}
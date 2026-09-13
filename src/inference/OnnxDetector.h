#ifndef ONNX_DETECTOR_H
#define ONNX_DETECTOR_H

#include <string>
#include <vector>
#include <onnxruntime_cxx_api.h>

class OnnxDetector {
public:
    OnnxDetector(const std::string& modelPath);
    ~OnnxDetector();

    bool loadModel(const std::string& modelPath);

    std::vector<float> inference(const std::vector<float>& inputTensor);
    
private:
    Ort::Env m_env;
    Ort::SessionOptions m_sessionOptions;
    std::unique_ptr<Ort::Session> m_session;
};

#endif
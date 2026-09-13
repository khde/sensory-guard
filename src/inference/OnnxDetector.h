#ifndef ONNX_DETECTOR_H
#define ONNX_DETECTOR_H

#include <string>
#include <vector>
#include <memory>
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

    // Input/Output names
    const char* m_inputName = "images";
    const char* m_outputName = "output0";

    // Shape
    const std::vector<int64_t> m_inputShape = {1, 3, 320, 320}; 
};

#endif
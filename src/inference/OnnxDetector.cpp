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
    if (!m_session) {
        std::cerr << "Inference failed: no model is loaded." << std::endl;
        return {};
    }

    const size_t expectedSize = 1 * 3 * 320 * 320;
    if (inputTensor.size() != expectedSize) {
        std::cerr << "Error: Input tensor size mismatch! Expected " << expectedSize 
                  << " elements, but got " << inputTensor.size() << std::endl;
        return std::vector<float>();
    }

    try {
        auto memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

        // Create ONNX tensor
        Ort::Value inputOrtTensor = Ort::Value::CreateTensor<float>(
            memoryInfo, 
            const_cast<float*>(inputTensor.data()), 
            inputTensor.size(), 
            m_inputShape.data(), 
            m_inputShape.size()
        );

        const char* inputNames[] = { m_inputName };
        const char* outputNames[] = { m_outputName };

        // Run inference
        std::vector<Ort::Value> outputTensors = m_session->Run(
            Ort::RunOptions{nullptr}, 
            inputNames, 
            &inputOrtTensor, 
            1, 
            outputNames, 
            1
        );

        float* floatRawData = outputTensors[0].GetTensorMutableData<float>();
        auto tensorInfo = outputTensors[0].GetTensorTypeAndShapeInfo();
        size_t outputSize = tensorInfo.GetElementCount();

        return std::vector<float>(floatRawData, floatRawData + outputSize);

    } catch (const Ort::Exception& e) {
        std::cerr << "Inference failed: " << e.what() << std::endl;
        return std::vector<float>();
    }
}
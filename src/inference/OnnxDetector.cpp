#include "OnnxDetector.h"

#include "inference/Labels.h"
#include "inference/HardwareDevices.h"

#ifdef SENSORYGUARD_USE_DIRECTML
#include <dml_provider_factory.h>
#endif

#include <iostream>
#include <algorithm>

OnnxDetector::OnnxDetector(
    const std::string& modelPath,
    const HardwareConfig &hardwareConfig,
    InitializationError &errorCode,
    std::string &errorMessage)
    : m_env(ORT_LOGGING_LEVEL_WARNING, "SensoryGuardInference") {
    m_sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

    if (hardwareConfig.cpuThreadCount < 1) {
        errorCode = InitializationError::InvalidConfiguration;
        errorMessage = "CPU thread count must be at least 1.";
        return;
    }

#ifdef SENSORYGUARD_USE_DIRECTML
    if (hardwareConfig.backend == InferenceBackend::DirectML) {
        int deviceIndex = hardwareConfig.deviceIndex;
        if (!hardwareConfig.deviceId.empty()) {
            deviceIndex = findDirectMLDeviceIndex(hardwareConfig.deviceId);
            if (deviceIndex < 0) {
                errorCode = InitializationError::DeviceUnavailable;
                errorMessage = "The selected DirectML GPU is no longer available.";
                return;
            }
        }
        m_sessionOptions.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
        m_sessionOptions.DisableMemPattern();
        OrtStatus *status = OrtSessionOptionsAppendExecutionProvider_DML(
            m_sessionOptions,
            deviceIndex);
        if (status) {
            errorCode = InitializationError::DeviceUnavailable;
            errorMessage = Ort::GetApi().GetErrorMessage(status);
            Ort::GetApi().ReleaseStatus(status);
            return;
        }
        std::cout << "Using DirectML GPU inference on device "
                  << deviceIndex << "." << std::endl;
    } else {
        m_sessionOptions.SetIntraOpNumThreads(hardwareConfig.cpuThreadCount);
    }
#else
    if (hardwareConfig.backend == InferenceBackend::DirectML) {
        errorCode = InitializationError::BackendUnavailable;
        errorMessage = "DirectML support is not available in this build.";
        return;
    }
    m_sessionOptions.SetIntraOpNumThreads(hardwareConfig.cpuThreadCount);
#endif

    try {
#ifdef _WIN32
        const std::wstring wideModelPath(modelPath.begin(), modelPath.end());
        m_session = std::make_unique<Ort::Session>(m_env, wideModelPath.c_str(), m_sessionOptions);
#else
        m_session = std::make_unique<Ort::Session>(m_env, modelPath.c_str(), m_sessionOptions);
#endif
        m_ready = true;
    } catch (const Ort::Exception& e) {
        errorCode = InitializationError::ModelLoadFailed;
        errorMessage = e.what();
    }
}

OnnxDetector::~OnnxDetector() = default;

bool OnnxDetector::isReady() const {
    return m_ready && m_session != nullptr;
}

std::vector<float> OnnxDetector::inference(const std::vector<float>& inputTensor) {
    if (!m_session) {
        std::cerr << "Inference failed: no model is loaded." << std::endl;
        return {};
    }

    const size_t expectedSize = 1 * 3 * 640 * 640;
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

        if (outputTensors.empty()) {
            std::cerr << "Inference returned no output." << std::endl;
            return {};
        }

        float* floatRawData = outputTensors[0].GetTensorMutableData<float>();
        auto tensorInfo = outputTensors[0].GetTensorTypeAndShapeInfo();
        size_t outputSize = tensorInfo.GetElementCount();

        return std::vector<float>(floatRawData, floatRawData + outputSize);

    } catch (const Ort::Exception& e) {
        std::cerr << "Inference failed: " << e.what() << std::endl;
        return std::vector<float>();
    }
}

std::vector<DetectionResult> OnnxDetector::detect(const cv::Mat &image, float confidenceThreshold) {
    if (image.empty()) {
        std::cerr << "Input image is empty!" << std::endl;
        return {};
    }

    int targetWidth = 640;
    int targetHeight = 640;
    int imgWidth = image.cols;
    int imgHeight = image.rows;

    float scale = std::min(static_cast<float>(targetWidth) / imgWidth, static_cast<float>(targetHeight) / imgHeight);
    int newWidth = static_cast<int>(imgWidth * scale);
    int newHeight = static_cast<int>(imgHeight * scale);
    int padX = (targetWidth - newWidth) / 2;
    int padY = (targetHeight - newHeight) / 2;

    cv::Mat resizedImage;
    cv::resize(image, resizedImage, cv::Size(newWidth, newHeight));

    // Center scaled image in YOLO-standard gray padding
    cv::Mat letterboxImage(targetHeight, targetWidth, CV_8UC3, cv::Scalar(114, 114, 114));
    resizedImage.copyTo(letterboxImage(cv::Rect(padX, padY, newWidth, newHeight)));

    // Convert from BGR to RGB
    cv::Mat rgbImage;
    cv::cvtColor(letterboxImage, rgbImage, cv::COLOR_BGR2RGB);

    // Convert to float and normalize to [0.0, 1.0]
    cv::Mat floatImage;
    rgbImage.convertTo(floatImage, CV_32FC3, 1.0 / 255.0);

    // Transform the HWC layout to CHW  for ONNX 
    std::vector<float> inputTensor(1 * 3 * targetHeight * targetWidth);
    int channelLength = targetHeight * targetWidth;
    for (int c = 0; c < 3; ++c) {
        for (int h = 0; h < targetHeight; ++h) {
            for (int w = 0; w < targetWidth; ++w) {
                // Read pixel value from the matrix and store it sequentially per channel
                inputTensor[c * channelLength + h * targetWidth + w] = floatImage.at<cv::Vec3f>(h, w)[c];
            }
        }
    }

    // Inference
    std::vector<float> outputRaw = inference(inputTensor);
    if (outputRaw.empty()) {
        return {};
    }

    // Process output
    const int numBoxes = 8400;
    const int numClasses = 18;

    std::vector<cv::Rect> bboxes;
    std::vector<float> scores;
    std::vector<int> classIds;

    for (int i = 0; i < numBoxes; ++i) {
        // Model output is channels, boxes
        float maxScore = outputRaw[4 * numBoxes + i];
        int bestClassId = 0;

        for (int classId = 1; classId < numClasses; ++classId) {
            float score = outputRaw[(4 + classId) * numBoxes + i];

            if (score > maxScore) {
                maxScore = score;
                bestClassId = classId;
            }
        }

        // Discard boxes with too low confidence
        if (maxScore >= confidenceThreshold) {
            // Extract coordinates in the 320x320 letterbox space
            float cx = outputRaw[0 * numBoxes + i];
            float cy = outputRaw[1 * numBoxes + i];
            float w  = outputRaw[2 * numBoxes + i];
            float h  = outputRaw[3 * numBoxes + i];

            // Convert from center X, center Y, width, height to X1, Y1 (top left)
            float x1 = cx - (w / 2.0f);
            float y1 = cy - (h / 2.0f);

            // Remove padding before mapping to the original image
            int origX1 = std::clamp(static_cast<int>((x1 - padX) / scale), 0, imgWidth);
            int origY1 = std::clamp(static_cast<int>((y1 - padY) / scale), 0, imgHeight);
            int origWidth  = std::clamp(static_cast<int>(w / scale), 0, imgWidth - origX1);
            int origHeight  = std::clamp(static_cast<int>(h / scale), 0, imgHeight - origY1);

            // Add the box only if it has a valid area
            if (origWidth > 0 && origHeight > 0) {
                bboxes.push_back(cv::Rect(origX1, origY1, origWidth, origHeight));
                scores.push_back(maxScore);
                classIds.push_back(bestClassId);
            }
        }
    }

    // NMS
    std::vector<int> indices;
    float nmsThreshold = 0.45f; // Intersection over Union
    cv::dnn::NMSBoxes(bboxes, scores, confidenceThreshold, nmsThreshold, indices);

    // Format results
    std::vector<DetectionResult> results;
    for (int idx : indices) {
        DetectionResult res;
        res.box.x1 = bboxes[idx].x;
        res.box.y1 = bboxes[idx].y;
        res.box.x2 = bboxes[idx].x + bboxes[idx].width;
        res.box.y2 = bboxes[idx].y + bboxes[idx].height;
        res.score = scores[idx];
        
        // Map the ID to labels
        if (classIds[idx] >= 0 && classIds[idx] < static_cast<int>(MODEL_LABELS.size())) {
            res.entity = MODEL_LABELS[classIds[idx]];
        } else {
            res.entity = "UNKNOWN";
        }

        results.push_back(res);
    }

    return results;
}
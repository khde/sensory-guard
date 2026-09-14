#include "OnnxDetector.h"

#include "inference/Labels.h"

#include <iostream>
#include <algorithm>

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

std::vector<DetectionResult> OnnxDetector::detect(const cv::Mat &image, float confidenceThreshold) {
    int targetWidth = 320;
    int targetHeight = 320;
    int imgWidth = image.cols;
    int imgHeight = image.rows;

    float scale = std::min(static_cast<float>(targetWidth) / imgWidth, static_cast<float>(targetHeight) / imgHeight);
    int newWidth = static_cast<int>(imgWidth * scale);
    int newHeight = static_cast<int>(imgHeight * scale);

    cv::Mat resizedImage;
    cv::resize(image, resizedImage, cv::Size(newWidth, newHeight));

    // Create black 320x320 image, place scaled image at top left
    cv::Mat letterboxImage = cv::Mat::zeros(targetHeight, targetWidth, CV_8UC3);
    resizedImage.copyTo(letterboxImage(cv::Rect(0, 0, newWidth, newHeight)));

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
    const int numBoxes = 2100;
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

            // Map the coordinates back to the original image size
            int origX1 = std::clamp(static_cast<int>(x1 / scale), 0, imgWidth);
            int origY1 = std::clamp(static_cast<int>(y1 / scale), 0, imgHeight);
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
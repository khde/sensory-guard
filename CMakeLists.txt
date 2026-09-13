cmake_minimum_required(VERSION 3.16)
project(SensorGuard VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(OpenCV REQUIRED CONFIG PATHS /usr/lib/x86_64-linux-gnu/cmake/opencv4)

find_path(ONNXRUNTIME_INCLUDE_DIR
    NAMES onnxruntime_cxx_api.h
    PATH_SUFFIXES onnxruntime
)
find_library(ONNXRUNTIME_LIBRARY NAMES onnxruntime)

if(NOT ONNXRUNTIME_INCLUDE_DIR OR NOT ONNXRUNTIME_LIBRARY)
    message(FATAL_ERROR "ONNX Runtime development files were not found")
endif()

add_executable(SensorGuard
    src/main.cpp
    src/gui/MainWindow.h
    src/gui/MainWindow.cpp
    src/gui/tabs/HomeTab.h
    src/gui/tabs/HomeTab.cpp
    src/gui/tabs/DetectionTab.h
    src/gui/tabs/DetectionTab.cpp
    src/gui/tabs/CustomizeTab.h
    src/gui/tabs/CustomizeTab.cpp
    src/gui/tabs/SettingsTab.h
    src/gui/tabs/SettingsTab.cpp
    src/backend/IShieldController.h
    src/backend/StubShieldController.h
    src/backend/StubShieldController.cpp
    src/inference/OnnxDetector.cpp
    src/inference/OnnxDetector.h
    src/inference/Detection.h
    src/inference/Labels.h
    src/platform/IScreenCapture.h
    src/platform/WindowsCapture.cpp
)

target_include_directories(SensorGuard PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${ONNXRUNTIME_INCLUDE_DIR}
)

target_link_libraries(SensorGuard PRIVATE
    Qt6::Widgets
    ${OpenCV_LIBS}
    ${ONNXRUNTIME_LIBRARY}
)

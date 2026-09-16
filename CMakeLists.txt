cmake_minimum_required(VERSION 3.16)
project(SensorGuard VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(OpenCV REQUIRED COMPONENTS core imgproc dnn CONFIG)

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
    src/gui/overlay/CensorOverlay.h
    src/gui/overlay/CensorOverlay.cpp
    src/engine/GuardEngine.h
    src/engine/EngineConfig.h
    src/engine/GuardEngine.cpp
    src/inference/OnnxDetector.cpp
    src/inference/OnnxDetector.h
    src/inference/Detection.h
    src/inference/Labels.h
    src/platform/IScreenCapture.h
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

if(WIN32)
    target_sources(SensorGuard PRIVATE
        src/platform/WindowsCapture.h
        src/platform/WindowsCapture.cpp
    )

    target_link_libraries(SensorGuard PRIVATE
        d3d11
        dxgi
    )

    if(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
        set(SENSORGUARD_VCPKG_ROOT "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
    elseif(CMAKE_PREFIX_PATH)
        list(GET CMAKE_PREFIX_PATH 0 SENSORGUARD_VCPKG_ROOT)
    else()
        message(FATAL_ERROR "Dependency prefix not found. Configure with the vcpkg toolchain or set CMAKE_PREFIX_PATH.")
    endif()

    add_custom_command(TARGET SensorGuard POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<IF:$<CONFIG:Debug>,${SENSORGUARD_VCPKG_ROOT}/debug/bin,${SENSORGUARD_VCPKG_ROOT}/bin>"
            "$<TARGET_FILE_DIR:SensorGuard>"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<IF:$<CONFIG:Debug>,${SENSORGUARD_VCPKG_ROOT}/debug/Qt6/plugins/platforms,${SENSORGUARD_VCPKG_ROOT}/Qt6/plugins/platforms>"
            "$<TARGET_FILE_DIR:SensorGuard>/platforms"
    )
endif()

if(UNIX AND NOT APPLE)
    find_package(X11 QUIET)
    if(X11_FOUND)
        target_compile_definitions(SensorGuard PRIVATE SENSORGUARD_HAS_X11)
        target_sources(SensorGuard PRIVATE
            src/platform/LinuxCapture.h
            src/platform/LinuxCapture.cpp
        )
        target_include_directories(SensorGuard PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(SensorGuard PRIVATE ${X11_LIBRARIES})
        if(X11_Xext_FOUND)
            target_link_libraries(SensorGuard PRIVATE ${X11_Xext_LIB})
        else()
            target_link_libraries(SensorGuard PRIVATE Xext)
        endif()
    endif()
endif()

cmake_minimum_required(VERSION 3.16)
project(SensorGuard VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(OpenCV REQUIRED COMPONENTS core imgproc dnn video CONFIG)

find_path(ONNXRUNTIME_INCLUDE_DIR
    NAMES onnxruntime_cxx_api.h
    PATH_SUFFIXES onnxruntime
)
find_library(ONNXRUNTIME_LIBRARY NAMES onnxruntime)

if(NOT ONNXRUNTIME_INCLUDE_DIR OR NOT ONNXRUNTIME_LIBRARY)
    message(FATAL_ERROR "ONNX Runtime development files were not found")
endif()

set(SENSORGUARD_ONNXRUNTIME_INCLUDE_DIR ${ONNXRUNTIME_INCLUDE_DIR})
set(SENSORGUARD_ONNXRUNTIME_LIBRARY ${ONNXRUNTIME_LIBRARY})

if(WIN32)
    option(SENSORGUARD_ENABLE_DIRECTML "Enable DirectML GPU inference on Windows" ON)
    if(SENSORGUARD_ENABLE_DIRECTML)
        include(ExternalProject)
        set(SENSORGUARD_DIRECTML_ROOT "${CMAKE_BINARY_DIR}/onnxruntime-directml")
        set(SENSORGUARD_DIRECTML_NATIVE_DIR "${SENSORGUARD_DIRECTML_ROOT}/runtimes/win-x64/native")
        set(SENSORGUARD_DIRECTML_INCLUDE_DIR "${SENSORGUARD_DIRECTML_ROOT}/build/native/include")

        ExternalProject_Add(SensorGuardDirectMLRuntime
            URL "https://www.nuget.org/api/v2/package/Microsoft.ML.OnnxRuntime.DirectML/1.24.4"
            SOURCE_DIR "${SENSORGUARD_DIRECTML_ROOT}"
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ""
            UPDATE_COMMAND ""
        )

        add_library(SensorGuardDirectML SHARED IMPORTED GLOBAL)
        set_target_properties(SensorGuardDirectML PROPERTIES
            IMPORTED_IMPLIB "${SENSORGUARD_DIRECTML_NATIVE_DIR}/onnxruntime.lib"
            IMPORTED_LOCATION "${SENSORGUARD_DIRECTML_NATIVE_DIR}/onnxruntime.dll"
        )
        add_dependencies(SensorGuardDirectML SensorGuardDirectMLRuntime)
        set(SENSORGUARD_ONNXRUNTIME_INCLUDE_DIR ${SENSORGUARD_DIRECTML_INCLUDE_DIR})
        set(SENSORGUARD_ONNXRUNTIME_LIBRARY SensorGuardDirectML)
    endif()
endif()

add_executable(SensorGuard
    src/main.cpp

    src/gui/MainWindow.h
    src/gui/MainWindow.cpp
    src/gui/AboutWindow.h
    src/gui/AboutWindow.cpp
    src/gui/tabs/HomeTab.h
    src/gui/tabs/HomeTab.cpp
    src/gui/tabs/DetectionTab.h
    src/gui/tabs/DetectionTab.cpp
    src/gui/tabs/CensorTab.h
    src/gui/tabs/CensorTab.cpp
    src/gui/tabs/SettingsTab.h
    src/gui/tabs/SettingsTab.cpp
    src/gui/overlay/CensorOverlay.h
    src/gui/overlay/CensorOverlay.cpp
    src/gui/overlay/CensorStyle.h
    src/gui/overlay/CensorStyle.cpp
    src/gui/widgets/ResponsiveTabWidget.h
    src/gui/widgets/ResponsiveTabWidget.cpp
    src/gui/theme/ThemeManager.h
    src/gui/theme/ThemeManager.cpp
    resources.qrc
    src/engine/GuardEngine.h
    src/engine/EngineConfig.h
    src/engine/GuardEngine.cpp
    src/inference/OnnxDetector.cpp
    src/inference/OnnxDetector.h
    src/inference/Detection.h
    src/inference/Labels.h
    src/inference/HardwareDevices.h
    src/inference/HardwareDevices.cpp
    src/config/UserSettings.h
    src/config/UserSettings.cpp
    src/platform/IScreenCapture.h
)

target_include_directories(SensorGuard PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SENSORGUARD_ONNXRUNTIME_INCLUDE_DIR}
)

target_link_libraries(SensorGuard PRIVATE
    Qt6::Widgets
    ${OpenCV_LIBS}
    ${SENSORGUARD_ONNXRUNTIME_LIBRARY}
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

    # Use std::min/std::max instead of macros defined in windows.h
    target_compile_definitions(SensorGuard PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)

    if(SENSORGUARD_ENABLE_DIRECTML)
        add_dependencies(SensorGuard SensorGuardDirectMLRuntime)
        target_compile_definitions(SensorGuard PRIVATE SENSORGUARD_USE_DIRECTML)
        add_custom_command(TARGET SensorGuard POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SENSORGUARD_DIRECTML_NATIVE_DIR}/onnxruntime.dll"
                "$<TARGET_FILE_DIR:SensorGuard>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SENSORGUARD_DIRECTML_NATIVE_DIR}/onnxruntime_providers_shared.dll"
                "$<TARGET_FILE_DIR:SensorGuard>"
        )
    endif()

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

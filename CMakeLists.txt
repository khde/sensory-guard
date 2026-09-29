cmake_minimum_required(VERSION 3.16)
project(SensoryGuard LANGUAGES CXX)

set(SENSORYGUARD_VERSION "0.0.0-dev" CACHE STRING "Application version")

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

set(SENSORYGUARD_ONNXRUNTIME_INCLUDE_DIR ${ONNXRUNTIME_INCLUDE_DIR})
set(SENSORYGUARD_ONNXRUNTIME_LIBRARY ${ONNXRUNTIME_LIBRARY})

if(WIN32)
    option(SENSORYGUARD_ENABLE_DIRECTML "Enable DirectML GPU inference on Windows" ON)
    if(SENSORYGUARD_ENABLE_DIRECTML)
        include(ExternalProject)
        set(SENSORYGUARD_DIRECTML_ROOT "${CMAKE_BINARY_DIR}/onnxruntime-directml")
        set(SENSORYGUARD_DIRECTML_NATIVE_DIR "${SENSORYGUARD_DIRECTML_ROOT}/runtimes/win-x64/native")
        set(SENSORYGUARD_DIRECTML_INCLUDE_DIR "${SENSORYGUARD_DIRECTML_ROOT}/build/native/include")

        ExternalProject_Add(SensoryGuardDirectMLRuntime
            URL "https://www.nuget.org/api/v2/package/Microsoft.ML.OnnxRuntime.DirectML/1.24.4"
            SOURCE_DIR "${SENSORYGUARD_DIRECTML_ROOT}"
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ""
            UPDATE_COMMAND ""
        )

        add_library(SensoryGuardDirectML SHARED IMPORTED GLOBAL)
        set_target_properties(SensoryGuardDirectML PROPERTIES
            IMPORTED_IMPLIB "${SENSORYGUARD_DIRECTML_NATIVE_DIR}/onnxruntime.lib"
            IMPORTED_LOCATION "${SENSORYGUARD_DIRECTML_NATIVE_DIR}/onnxruntime.dll"
        )
        add_dependencies(SensoryGuardDirectML SensoryGuardDirectMLRuntime)
        set(SENSORYGUARD_ONNXRUNTIME_INCLUDE_DIR ${SENSORYGUARD_DIRECTML_INCLUDE_DIR})
        set(SENSORYGUARD_ONNXRUNTIME_LIBRARY SensoryGuardDirectML)
    endif()
endif()

add_executable(SensoryGuard
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
    src/platform/DisplayDevices.h
    src/platform/DisplayDevices.cpp
    src/platform/ToggleHotkey.h
    src/platform/ToggleHotkey.cpp
)

target_include_directories(SensoryGuard PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SENSORYGUARD_ONNXRUNTIME_INCLUDE_DIR}
)

target_compile_definitions(SensoryGuard PRIVATE
    SENSORYGUARD_VERSION="${SENSORYGUARD_VERSION}"
)

target_link_libraries(SensoryGuard PRIVATE
    Qt6::Widgets
    ${OpenCV_LIBS}
    ${SENSORYGUARD_ONNXRUNTIME_LIBRARY}
)

if(WIN32)
    target_sources(SensoryGuard PRIVATE
        src/platform/WindowsCapture.h
        src/platform/WindowsCapture.cpp
    )

    target_link_libraries(SensoryGuard PRIVATE
        d3d11
        dxgi
    )

    # Use std::min/std::max instead of macros defined in windows.h
    target_compile_definitions(SensoryGuard PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)

    if(SENSORYGUARD_ENABLE_DIRECTML)
        add_dependencies(SensoryGuard SensoryGuardDirectMLRuntime)
        target_compile_definitions(SensoryGuard PRIVATE SENSORYGUARD_USE_DIRECTML)
    endif()

    if(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
        set(SENSORYGUARD_VCPKG_ROOT "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
    elseif(CMAKE_PREFIX_PATH)
        list(GET CMAKE_PREFIX_PATH 0 SENSORYGUARD_VCPKG_ROOT)
    else()
        message(FATAL_ERROR "Dependency prefix not found. Configure with the vcpkg toolchain or set CMAKE_PREFIX_PATH.")
    endif()

    add_custom_command(TARGET SensoryGuard POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<IF:$<CONFIG:Debug>,${SENSORYGUARD_VCPKG_ROOT}/debug/bin,${SENSORYGUARD_VCPKG_ROOT}/bin>"
            "$<TARGET_FILE_DIR:SensoryGuard>"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<IF:$<CONFIG:Debug>,${SENSORYGUARD_VCPKG_ROOT}/debug/Qt6/plugins/platforms,${SENSORYGUARD_VCPKG_ROOT}/Qt6/plugins/platforms>"
            "$<TARGET_FILE_DIR:SensoryGuard>/platforms"
    )

    if(SENSORYGUARD_ENABLE_DIRECTML)
        add_custom_command(TARGET SensoryGuard POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SENSORYGUARD_DIRECTML_NATIVE_DIR}/onnxruntime.dll"
                "$<TARGET_FILE_DIR:SensoryGuard>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SENSORYGUARD_DIRECTML_NATIVE_DIR}/onnxruntime_providers_shared.dll"
                "$<TARGET_FILE_DIR:SensoryGuard>"
        )
    endif()
endif()

if(UNIX AND NOT APPLE)
    find_package(X11 QUIET)
    if(X11_FOUND)
        target_compile_definitions(SensoryGuard PRIVATE SENSORYGUARD_HAS_X11)
        target_sources(SensoryGuard PRIVATE
            src/platform/LinuxCapture.h
            src/platform/LinuxCapture.cpp
        )
        target_include_directories(SensoryGuard PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(SensoryGuard PRIVATE ${X11_LIBRARIES})
        if(X11_Xext_FOUND)
            target_link_libraries(SensoryGuard PRIVATE ${X11_Xext_LIB})
        else()
            target_link_libraries(SensoryGuard PRIVATE Xext)
        endif()
    endif()
endif()

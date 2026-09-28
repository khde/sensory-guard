# Sensor Guard

Sensor Guard is an offline, real-time NSFW screen censor desktop application that uses an on-device machine learning model to detect and censor sensitive content live on the screen.

## Features

- Live screen capture, content detection, and on-screen censoring
- Selectable content categories, sensitivity and processing rate
- Different censor styles: Solid, blur, and pixelation
- CPU inference, with DirectML GPU support on Windows

## Dependencies

The application is built with:

- Qt
- OpenCV
- ONNX Runtime

## Linux

Currently the Linux screencapture is only supported on X11.

### Limitation

X11 and Wayland currently have no equivalent to Windows' `SetWindowDisplayAffinity`, which keeps the censor overlay out of screen capture. As a result, the model sees the censor overlay instead of the underlying content, causing feedback flickering, so censoring does not work reliably on Linux.

### Install dependencies

Install the compiler, build tools, Qt, OpenCV, ONNX Runtime and X11 development packages:

```bash
sudo apt update
sudo apt install -y qt6-base-dev libopencv-dev libx11-dev libxext-dev libonnxruntime-dev
```

### Build

Run these commands from the repository root:

```bash
cmake -S . -B build
cmake --build build
```

### Run

```bash
./build/SensorGuard
```

## Windows

### Install dependencies

From the repository root, install the manifest dependencies for the 64-bit Windows triplet:

```powershell
& "$env:VCPKG_ROOT\vcpkg.exe" install --triplet x64-windows
```

This installs Qt, OpenCV, and ONNX Runtime from [vcpkg.json](vcpkg.json).

### Configure and build

Configure CMake with the vcpkg toolchain file:

```powershell
cmake -S . -B build `
	-DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake" `
	-DVCPKG_TARGET_TRIPLET=x64-windows
```

Build the Release configuration:

```powershell
cmake --build build --config Release --parallel
```

### Run

```powershell
.\build\Release\SensorGuard.exe
```

## License

Sensor Guard is licensed under the MIT License. See [LICENSE](LICENSE) for the full license text.

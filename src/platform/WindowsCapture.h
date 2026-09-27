#ifndef WINDOWS_CAPTURE_H
#define WINDOWS_CAPTURE_H

#ifdef _WIN32

#include "platform/IScreenCapture.h"
#include "platform/DisplayDevices.h"

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

class WindowsScreenCapture final : public IScreenCapture {
public:
    explicit WindowsScreenCapture(const DisplayDescriptor &display);
    ~WindowsScreenCapture() override = default;

    bool isAvailable() const override;
    bool captureFrame(cv::Mat &frame) override;

private:
    bool initialize();
    bool createStagingTexture(const D3D11_TEXTURE2D_DESC &sourceDescription);
    void releaseDuplication();

    DisplayDescriptor m_display;
    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
    Microsoft::WRL::ComPtr<IDXGIOutputDuplication> m_duplication;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_stagingTexture;
};

#endif

#endif
#ifndef WINDOWS_CAPTURE_H
#define WINDOWS_CAPTURE_H

#ifdef _WIN32

#include "platform/IScreenCapture.h"

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

class WindowsScreenCapture final : public IScreenCapture {
public:
    explicit WindowsScreenCapture(unsigned int outputIndex = 0);
    ~WindowsScreenCapture() override = default;

    bool isAvailable() const override;
    bool captureFrame(cv::Mat &frame) override;

private:
    bool initialize(unsigned int outputIndex);
    bool createStagingTexture(const D3D11_TEXTURE2D_DESC &sourceDescription);
    void releaseDuplication();

    unsigned int m_outputIndex = 0;
    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
    Microsoft::WRL::ComPtr<IDXGIOutputDuplication> m_duplication;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_stagingTexture;
};

#endif

#endif
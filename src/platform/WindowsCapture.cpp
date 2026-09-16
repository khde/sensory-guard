#include "WindowsCapture.h"

#ifdef _WIN32

#include <cstring>

using Microsoft::WRL::ComPtr;

WindowsScreenCapture::WindowsScreenCapture(unsigned int outputIndex) {
    initialize(outputIndex);
}

bool WindowsScreenCapture::initialize(unsigned int outputIndex) {
    releaseDuplication();

    ComPtr<IDXGIFactory1> factory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
        return false;

    ComPtr<IDXGIAdapter1> adapter;
    if (factory->EnumAdapters1(0, &adapter) == DXGI_ERROR_NOT_FOUND)
        return false;

    ComPtr<IDXGIOutput> output;
    if (adapter->EnumOutputs(outputIndex, &output) != S_OK)
        return false;

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0
    };
    D3D_FEATURE_LEVEL selectedFeatureLevel;
    if (FAILED(D3D11CreateDevice(
            adapter.Get(),
            D3D_DRIVER_TYPE_UNKNOWN,
            nullptr,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &m_device,
            &selectedFeatureLevel,
            &m_context))) {
        return false;
    }

    ComPtr<IDXGIOutput1> output1;
    if (FAILED(output.As(&output1)))
        return false;

    return SUCCEEDED(output1->DuplicateOutput(m_device.Get(), &m_duplication));
}

void WindowsScreenCapture::releaseDuplication() {
    m_stagingTexture.Reset();
    m_duplication.Reset();
    m_context.Reset();
    m_device.Reset();
}

bool WindowsScreenCapture::createStagingTexture(
    const D3D11_TEXTURE2D_DESC &sourceDescription) {
    D3D11_TEXTURE2D_DESC stagingDescription = sourceDescription;
    stagingDescription.Usage = D3D11_USAGE_STAGING;
    stagingDescription.BindFlags = 0;
    stagingDescription.MiscFlags = 0;
    stagingDescription.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    return SUCCEEDED(m_device->CreateTexture2D(
        &stagingDescription, nullptr, &m_stagingTexture));
}

bool WindowsScreenCapture::captureFrame(cv::Mat &frame) {
    if (!m_duplication || !m_context)
        return false;

    DXGI_OUTDUPL_FRAME_INFO frameInfo{};
    ComPtr<IDXGIResource> desktopResource;
    HRESULT result = m_duplication->AcquireNextFrame(
        100, &frameInfo, &desktopResource);

    if (result == DXGI_ERROR_WAIT_TIMEOUT)
        return false;

    if (result == DXGI_ERROR_ACCESS_LOST) {
        releaseDuplication();
        return false;
    }

    if (FAILED(result))
        return false;

    ComPtr<ID3D11Texture2D> desktopTexture;
    if (FAILED(desktopResource.As(&desktopTexture))) {
        m_duplication->ReleaseFrame();
        return false;
    }

    D3D11_TEXTURE2D_DESC textureDescription{};
    desktopTexture->GetDesc(&textureDescription);
    if (!m_stagingTexture && !createStagingTexture(textureDescription)) {
        m_duplication->ReleaseFrame();
        return false;
    }

    m_context->CopyResource(m_stagingTexture.Get(), desktopTexture.Get());

    D3D11_MAPPED_SUBRESOURCE mapped{};
    if (FAILED(m_context->Map(
            m_stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped))) {
        m_duplication->ReleaseFrame();
        return false;
    }

    frame.create(
        static_cast<int>(textureDescription.Height),
        static_cast<int>(textureDescription.Width),
        CV_8UC4);
    for (int row = 0; row < frame.rows; ++row) {
        std::memcpy(
            frame.ptr(row),
            static_cast<const unsigned char *>(mapped.pData) + row * mapped.RowPitch,
            static_cast<size_t>(frame.cols) * frame.elemSize());
    }

    m_context->Unmap(m_stagingTexture.Get(), 0);
    m_duplication->ReleaseFrame();
    return true;
}

#endif
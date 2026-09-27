#include "HardwareDevices.h"

#ifdef _WIN32
#include <dxgi1_2.h>
#include <cstdio>
#include <windows.h>
#include <wrl/client.h>
#endif

namespace {
#ifdef _WIN32
std::string utf8FromWide(const wchar_t *value) {
    if (value == nullptr || *value == L'\0')
        return {};

    const int requiredSize = WideCharToMultiByte(CP_UTF8, 0, value, -1, nullptr, 0, nullptr, nullptr);

    if (requiredSize <= 1){
        return {};
    }

    std::string result(static_cast<size_t>(requiredSize), '\0');
    WideCharToMultiByte(CP_UTF8, 0, value, -1, result.data(), requiredSize, nullptr, nullptr);
    result.pop_back();
    return result;
}

std::string idFromLuid(const LUID &luid) {
    char value[17]{};
    std::snprintf(value,sizeof(value), "%08lX%08lX", static_cast<unsigned long>(luid.HighPart), static_cast<unsigned long>(luid.LowPart));
    return value;
}
#endif
}

std::vector<HardwareDeviceInfo> enumerateDirectMLDevices() {
    std::vector<HardwareDeviceInfo> devices;

#ifdef _WIN32
    using Microsoft::WRL::ComPtr;

    ComPtr<IDXGIFactory1> factory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
        return devices;

    for (UINT adapterIndex = 0;; ++adapterIndex) {
        ComPtr<IDXGIAdapter1> adapter;
        if (factory->EnumAdapters1(adapterIndex, &adapter) == DXGI_ERROR_NOT_FOUND)
            break;

        DXGI_ADAPTER_DESC1 description{};
        if (FAILED(adapter->GetDesc1(&description)))
            continue;
        if (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            continue;

        devices.push_back({
            static_cast<int>(adapterIndex),
            idFromLuid(description.AdapterLuid),
            utf8FromWide(description.Description)});
    }
#endif

    return devices;
}

int findDirectMLDeviceIndex(const std::string &deviceId) {
    for (const HardwareDeviceInfo &device : enumerateDirectMLDevices()) {
        if (device.id == deviceId)
            return device.index;
    }
    return -1;
}

#include "audio_monitor.h"
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <functiondiscoverykeys_devpkey.h>
#include <cmath>

#pragma comment(lib, "ole32.lib")

AudioMonitor::AudioMonitor() 
    : initialized_(false), last_audio_state_(false), last_check_time_(0) {
}

AudioMonitor::~AudioMonitor() {
    Cleanup();
}

bool AudioMonitor::Initialize() {
    if (initialized_) {
        return true;
    }
    
    // 初始化COM（如果尚未初始化）
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        return false;
    }
    
    initialized_ = true;
    return true;
}

void AudioMonitor::Cleanup() {
    if (initialized_) {
        CoUninitialize();
        initialized_ = false;
    }
}

bool AudioMonitor::HasAudioActivity() {
    return GetAudioActivity();
}

bool AudioMonitor::GetAudioActivity() {
    DWORD current_time = GetTickCount();
    
    // 使用缓存，避免频繁检测
    if (current_time - last_check_time_ < CHECK_INTERVAL_MS) {
        return last_audio_state_;
    }
    
    last_check_time_ = current_time;
    last_audio_state_ = CheckAudioActivityInternal();
    return last_audio_state_;
}

bool AudioMonitor::CheckAudioActivityInternal() {
    if (!initialized_) {
        if (!Initialize()) {
            return false;
        }
    }
    
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioMeterInformation* pMeterInfo = NULL;
    
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        NULL,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&pEnumerator
    );
    
    if (FAILED(hr)) {
        return false;
    }
    
    // 获取默认音频渲染设备
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (FAILED(hr)) {
        pEnumerator->Release();
        return false;
    }
    
    // 获取音频计量信息
    hr = pDevice->Activate(
        __uuidof(IAudioMeterInformation),
        CLSCTX_ALL,
        NULL,
        (void**)&pMeterInfo
    );
    
    if (FAILED(hr)) {
        pDevice->Release();
        pEnumerator->Release();
        return false;
    }
    
    // 获取峰值音量（0.0 - 1.0）
    float peak_value = 0.0f;
    hr = pMeterInfo->GetPeakValue(&peak_value);
    
    // 清理资源
    pMeterInfo->Release();
    pDevice->Release();
    pEnumerator->Release();
    
    if (FAILED(hr)) {
        return false;
    }
    
    // 如果峰值音量超过阈值（0.01，即1%），认为有音频活动
    // 这个阈值可以根据需要调整
    const float AUDIO_THRESHOLD = 0.01f;
    return peak_value > AUDIO_THRESHOLD;
}




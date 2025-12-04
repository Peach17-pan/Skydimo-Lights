#pragma once

#include <windows.h>
#include <string>

/**
 * 音频监控器类
 * 负责检测系统是否有明显音频输出
 */
class AudioMonitor {
public:
    AudioMonitor();
    ~AudioMonitor();
    
    /**
     * 初始化音频监控
     * @return 是否初始化成功
     */
    bool Initialize();
    
    /**
     * 清理资源
     */
    void Cleanup();
    
    /**
     * 检测当前是否有音频活动
     * @return true表示有音频输出，false表示无音频输出
     */
    bool HasAudioActivity();
    
    /**
     * 获取音频活动状态（带缓存，避免频繁检测）
     * @return true表示有音频输出，false表示无音频输出
     */
    bool GetAudioActivity();

private:
    bool initialized_;
    bool last_audio_state_;
    DWORD last_check_time_;
    static const DWORD CHECK_INTERVAL_MS = 500;  // 每500ms检测一次
    
    /**
     * 使用Windows API检测音频活动
     * 通过检查默认音频设备的音量级别来判断
     */
    bool CheckAudioActivityInternal();
};




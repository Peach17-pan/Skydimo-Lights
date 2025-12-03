#pragma once

#include <windows.h>
#include <string>
#include <optional>

/**
 * 窗口信息结构体
 */
struct WindowInfo {
    std::string process_name;      // 可执行文件名
    std::string window_title;       // 窗口标题
    bool is_near_fullscreen;        // 是否接近全屏
    DWORD process_id;               // 进程ID
    std::optional<std::string> executable_path;  // 可执行文件路径（可选）
};

/**
 * 窗口监控器类
 * 负责获取前台窗口信息
 */
class WindowMonitor {
public:
    WindowMonitor();
    
    /**
     * 获取当前前台窗口信息
     * @return WindowInfo对象，如果获取失败则返回std::nullopt
     */
    std::optional<WindowInfo> GetForegroundWindowInfo();

private:
    int screen_width_;
    int screen_height_;
    double fullscreen_threshold_;  // 95%以上视为接近全屏
    
    /**
     * 检查窗口是否接近全屏
     * @param hwnd 窗口句柄
     * @return 如果窗口占据屏幕95%以上则返回true
     */
    bool CheckNearFullscreen(HWND hwnd);
    
    /**
     * 获取进程的可执行文件名
     * @param process_id 进程ID
     * @return 进程名，如果获取失败则返回空字符串
     */
    std::string GetProcessName(DWORD process_id);
    
    /**
     * 获取进程的可执行文件路径
     * @param process_id 进程ID
     * @return 可执行文件路径，如果获取失败则返回std::nullopt
     */
    std::optional<std::string> GetExecutablePath(DWORD process_id);
    
    /**
     * 将宽字符串转换为UTF-8字符串
     */
    std::string WideToUtf8(const std::wstring& wstr);
    
    /**
     * 将字符串转换为小写
     */
    std::string ToLower(const std::string& str);
};


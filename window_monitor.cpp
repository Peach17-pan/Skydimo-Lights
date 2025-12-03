#include "window_monitor.h"
#include <psapi.h>
#include <tlhelp32.h>
#include <algorithm>
#include <cctype>

#pragma comment(lib, "psapi.lib")

WindowMonitor::WindowMonitor() {
    screen_width_ = GetSystemMetrics(SM_CXSCREEN);
    screen_height_ = GetSystemMetrics(SM_CYSCREEN);
    fullscreen_threshold_ = 0.95;  // 95%以上视为接近全屏
}

std::optional<WindowInfo> WindowMonitor::GetForegroundWindowInfo() {
    try {
        // 获取前台窗口句柄
        HWND hwnd = GetForegroundWindow();
        if (!hwnd) {
            return std::nullopt;
        }
        
        // 获取窗口标题
        wchar_t window_title_w[256] = {0};
        GetWindowTextW(hwnd, window_title_w, sizeof(window_title_w) / sizeof(wchar_t));
        std::string window_title = WideToUtf8(window_title_w);
        
        // 获取进程ID
        DWORD process_id = 0;
        GetWindowThreadProcessId(hwnd, &process_id);
        if (process_id == 0) {
            return std::nullopt;
        }
        
        // 获取进程名
        std::string process_name = GetProcessName(process_id);
        if (process_name.empty()) {
            process_name = "Process_" + std::to_string(process_id);
        }
        
        // 获取可执行文件路径（可选）
        std::optional<std::string> executable_path = GetExecutablePath(process_id);
        
        // 检查是否接近全屏
        bool is_near_fullscreen = CheckNearFullscreen(hwnd);
        
        WindowInfo info;
        info.process_name = process_name;
        info.window_title = window_title;
        info.is_near_fullscreen = is_near_fullscreen;
        info.process_id = process_id;
        info.executable_path = executable_path;
        
        return info;
    } catch (...) {
        return std::nullopt;
    }
}

bool WindowMonitor::CheckNearFullscreen(HWND hwnd) {
    try {
        RECT rect;
        if (!GetWindowRect(hwnd, &rect)) {
            return false;
        }
        
        int window_width = rect.right - rect.left;
        int window_height = rect.bottom - rect.top;
        
        // 计算窗口占屏幕的比例
        double width_ratio = static_cast<double>(window_width) / screen_width_;
        double height_ratio = static_cast<double>(window_height) / screen_height_;
        
        // 如果宽度和高度都超过阈值，则认为接近全屏
        return width_ratio >= fullscreen_threshold_ && height_ratio >= fullscreen_threshold_;
    } catch (...) {
        return false;
    }
}

std::string WindowMonitor::GetProcessName(DWORD process_id) {
    // 方法1: 使用QueryFullProcessImageName（Windows Vista+）
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);
    if (hProcess != NULL) {
        wchar_t process_path[MAX_PATH] = {0};
        DWORD size = MAX_PATH;
        
        // 尝试使用QueryFullProcessImageNameW
        if (QueryFullProcessImageNameW(hProcess, 0, process_path, &size)) {
            CloseHandle(hProcess);
            std::string path = WideToUtf8(process_path);
            // 提取文件名
            size_t last_slash = path.find_last_of("\\/");
            if (last_slash != std::string::npos) {
                return path.substr(last_slash + 1);
            }
            return path;
        }
        CloseHandle(hProcess);
    }
    
    // 方法2: 使用CreateToolhelp32Snapshot作为后备
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        
        if (Process32FirstW(hSnapshot, &pe32)) {
            do {
                if (pe32.th32ProcessID == process_id) {
                    CloseHandle(hSnapshot);
                    return WideToUtf8(pe32.szExeFile);
                }
            } while (Process32NextW(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
    
    return "";
}

std::optional<std::string> WindowMonitor::GetExecutablePath(DWORD process_id) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);
    if (hProcess == NULL) {
        return std::nullopt;
    }
    
    wchar_t process_path[MAX_PATH] = {0};
    DWORD size = MAX_PATH;
    
    if (QueryFullProcessImageNameW(hProcess, 0, process_path, &size)) {
        CloseHandle(hProcess);
        return WideToUtf8(process_path);
    }
    
    CloseHandle(hProcess);
    return std::nullopt;
}

std::string WindowMonitor::WideToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) {
        return "";
    }
    
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (size_needed <= 0) {
        return "";
    }
    
    std::string result(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size_needed, NULL, NULL);
    
    // 移除末尾的null字符
    if (!result.empty() && result.back() == '\0') {
        result.pop_back();
    }
    
    return result;
}

std::string WindowMonitor::ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}


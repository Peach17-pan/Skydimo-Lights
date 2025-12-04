#include "window_monitor.h"
#include "app_classifier.h"
#include "rule_engine.h"
#include "audio_monitor.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <atomic>
#include <csignal>
#include <ctime>
#include <windows.h>

// 全局变量用于信号处理
std::atomic<bool> g_running(true);

// 信号处理函数
void SignalHandler(int signal) {
    if (signal == SIGINT || signal == CTRL_C_EVENT) {
        g_running = false;
    }
}

// Windows控制台信号处理
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_CLOSE_EVENT) {
        g_running = false;
        return TRUE;
    }
    return FALSE;
}

/**
 * 获取当前时间戳字符串
 */
std::string GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::tm tm_buf;
    localtime_s(&tm_buf, &time_t);
    
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return oss.str();
}

/**
 * 获取当前时间（小时:分钟）
 */
std::string GetCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::tm tm_buf;
    localtime_s(&tm_buf, &time_t);
    
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << tm_buf.tm_hour << ":"
        << std::setfill('0') << std::setw(2) << tm_buf.tm_min;
    
    return oss.str();
}

/**
 * 获取星期几（周一~周日）
 */
std::string GetWeekday() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::tm tm_buf;
    localtime_s(&tm_buf, &time_t);
    
    // tm_wday: 0=周日, 1=周一, ..., 6=周六
    const char* weekdays[] = {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};
    int weekday_index = tm_buf.tm_wday;
    
    return std::string(weekdays[weekday_index]);
}

/**
 * 判断是否是工作日（周一～周五）
 * @return true表示工作日，false表示周末
 */
bool IsWeekday() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::tm tm_buf;
    localtime_s(&tm_buf, &time_t);
    
    // tm_wday: 0=周日, 1=周一, ..., 6=周六
    // 工作日：1-5（周一到周五）
    return tm_buf.tm_wday >= 1 && tm_buf.tm_wday <= 5;
}

/**
 * 获取用户空闲时间（从上次键盘/鼠标操作起，已经空闲了多少分钟）
 * @return 空闲时间（分钟），如果获取失败则返回-1
 */
double GetUserIdleMinutes() {
    LASTINPUTINFO last_input_info;
    last_input_info.cbSize = sizeof(LASTINPUTINFO);
    
    if (!GetLastInputInfo(&last_input_info)) {
        return -1.0;  // 获取失败
    }
    
    // 获取当前系统运行时间（毫秒）
    DWORD current_tick = GetTickCount();
    
    // 计算空闲时间（毫秒）
    DWORD idle_time_ms = current_tick - last_input_info.dwTime;
    
    // 转换为分钟
    double idle_minutes = static_cast<double>(idle_time_ms) / 60000.0;
    
    return idle_minutes;
}

/**
 * CPU使用率监控类
 */
class CpuMonitor {
private:
    FILETIME last_idle_time_;
    FILETIME last_kernel_time_;
    FILETIME last_user_time_;
    bool initialized_;

public:
    CpuMonitor() : initialized_(false) {
        last_idle_time_.dwLowDateTime = 0;
        last_idle_time_.dwHighDateTime = 0;
        last_kernel_time_.dwLowDateTime = 0;
        last_kernel_time_.dwHighDateTime = 0;
        last_user_time_.dwLowDateTime = 0;
        last_user_time_.dwHighDateTime = 0;
    }

    /**
     * 获取CPU使用率（0-100%）
     */
    double GetCpuUsage() {
        FILETIME idle_time, kernel_time, user_time;
        
        if (!GetSystemTimes(&idle_time, &kernel_time, &user_time)) {
            return -1.0;  // 获取失败
        }

        if (!initialized_) {
            // 第一次调用，只记录时间，不计算使用率
            last_idle_time_ = idle_time;
            last_kernel_time_ = kernel_time;
            last_user_time_ = user_time;
            initialized_ = true;
            return 0.0;  // 第一次返回0
        }

        // 计算时间差
        ULARGE_INTEGER idle_diff, kernel_diff, user_diff;
        ULARGE_INTEGER last_idle, last_kernel, last_user;
        ULARGE_INTEGER current_idle, current_kernel, current_user;

        last_idle.LowPart = last_idle_time_.dwLowDateTime;
        last_idle.HighPart = last_idle_time_.dwHighDateTime;
        last_kernel.LowPart = last_kernel_time_.dwLowDateTime;
        last_kernel.HighPart = last_kernel_time_.dwHighDateTime;
        last_user.LowPart = last_user_time_.dwLowDateTime;
        last_user.HighPart = last_user_time_.dwHighDateTime;

        current_idle.LowPart = idle_time.dwLowDateTime;
        current_idle.HighPart = idle_time.dwHighDateTime;
        current_kernel.LowPart = kernel_time.dwLowDateTime;
        current_kernel.HighPart = kernel_time.dwHighDateTime;
        current_user.LowPart = user_time.dwLowDateTime;
        current_user.HighPart = user_time.dwHighDateTime;

        idle_diff.QuadPart = current_idle.QuadPart - last_idle.QuadPart;
        kernel_diff.QuadPart = current_kernel.QuadPart - last_kernel.QuadPart;
        user_diff.QuadPart = current_user.QuadPart - last_user.QuadPart;

        // 总CPU时间 = 内核时间 + 用户时间
        ULONGLONG total_time = kernel_diff.QuadPart + user_diff.QuadPart;
        
        if (total_time == 0) {
            // 时间差为0，返回上一次的值或0
            return 0.0;
        }

        // CPU使用率 = (总时间 - 空闲时间) / 总时间 * 100
        ULONGLONG used_time = total_time - idle_diff.QuadPart;
        double cpu_usage = (static_cast<double>(used_time) / static_cast<double>(total_time)) * 100.0;

        // 更新上一次的时间
        last_idle_time_ = idle_time;
        last_kernel_time_ = kernel_time;
        last_user_time_ = user_time;

        // 确保返回值在0-100范围内
        if (cpu_usage < 0.0) cpu_usage = 0.0;
        if (cpu_usage > 100.0) cpu_usage = 100.0;

        return cpu_usage;
    }
};

/**
 * 初始化规则引擎，添加示例规则
 */
void InitializeRules(RuleEngine& rule_engine) {
    Rule rule;
    
    // 规则1: 夜间弱光模式（23:00-07:00，优先级10）
    rule = Rule();
    rule.priority = 10;
    rule.target_mode = LightMode::NIGHT_DIM;
    Condition time_condition;
    time_condition.type = ConditionType::TIME_RANGE;
    time_condition.time_range = TimeRange(23, 0, 7, 0);  // 23:00-07:00
    rule.conditions.push_back(time_condition);
    rule_engine.AddRule(rule);
    
    // 规则2: 空闲时间超过10分钟，关闭灯光（优先级9）
    rule = Rule();
    rule.priority = 9;
    rule.target_mode = LightMode::OFF;
    Condition idle_condition;
    idle_condition.type = ConditionType::IDLE_THRESHOLD;
    idle_condition.idle_threshold = 10.0;  // 10分钟
    idle_condition.idle_greater_than = true;  // >= 10分钟
    rule.conditions.push_back(idle_condition);
    rule_engine.AddRule(rule);
    
    // 规则3: 游戏类应用，使用游戏/屏幕同步模式（优先级8）
    rule = Rule();
    rule.priority = 8;
    rule.target_mode = LightMode::GAME_SCREENSYNC;
    Condition game_condition;
    game_condition.type = ConditionType::APP_CATEGORY;
    game_condition.app_category = AppCategory::GAME;
    rule.conditions.push_back(game_condition);
    rule_engine.AddRule(rule);
    
    // 规则4: 视频类应用，使用影视模式（优先级7）
    rule = Rule();
    rule.priority = 7;
    rule.target_mode = LightMode::VIDEO_CINEMATIC;
    Condition video_condition;
    video_condition.type = ConditionType::APP_CATEGORY;
    video_condition.app_category = AppCategory::VIDEO;
    rule.conditions.push_back(video_condition);
    rule_engine.AddRule(rule);
    
    // 规则5: 音乐类应用，使用音乐律动模式（优先级6）
    rule = Rule();
    rule.priority = 6;
    rule.target_mode = LightMode::MUSIC;
    Condition music_condition;
    music_condition.type = ConditionType::APP_CATEGORY;
    music_condition.app_category = AppCategory::MUSIC;
    rule.conditions.push_back(music_condition);
    rule_engine.AddRule(rule);
    
    // 规则6: 开发/编程类应用，使用办公/写代码模式（优先级5）
    rule = Rule();
    rule.priority = 5;
    rule.target_mode = LightMode::WORK_CODING;
    Condition dev_condition;
    dev_condition.type = ConditionType::APP_CATEGORY;
    dev_condition.app_category = AppCategory::DEVELOPMENT;
    rule.conditions.push_back(dev_condition);
    rule_engine.AddRule(rule);
    
    // 规则7: 文档/办公类应用，使用办公/写代码模式（优先级4）
    rule = Rule();
    rule.priority = 4;
    rule.target_mode = LightMode::WORK_CODING;
    Condition doc_condition;
    doc_condition.type = ConditionType::APP_CATEGORY;
    doc_condition.app_category = AppCategory::DOCUMENT;
    rule.conditions.push_back(doc_condition);
    rule_engine.AddRule(rule);
    
    // 规则8: CPU使用率超过80%，使用游戏/屏幕同步模式（优先级3）
    rule = Rule();
    rule.priority = 3;
    rule.target_mode = LightMode::GAME_SCREENSYNC;
    Condition cpu_condition;
    cpu_condition.type = ConditionType::CPU_THRESHOLD;
    cpu_condition.cpu_threshold = 80.0;  // 80%
    cpu_condition.cpu_greater_than = true;  // > 80%
    rule.conditions.push_back(cpu_condition);
    rule_engine.AddRule(rule);
    
    // 规则9: 有音频活动且非游戏场景，使用音乐律动模式（优先级2.5）
    // 实现方式：由于游戏、视频、音乐应用会被更高优先级规则覆盖，
    // 这个规则主要针对浏览器、未知应用等非游戏场景
    // 优先级设置为2，高于工作日规则（1）和周末规则（0），确保音频规则优先
    rule = Rule();
    rule.priority = 2;
    rule.target_mode = LightMode::MUSIC;
    Condition audio_condition;
    audio_condition.type = ConditionType::AUDIO_ACTIVITY;
    audio_condition.audio_activity = true;  // 有音频
    rule.conditions.push_back(audio_condition);
    // 注意：由于游戏规则（优先级8）更高，游戏场景会被覆盖
    // 视频规则（优先级7）和音乐规则（优先级6）也会覆盖
    // 所以这个规则主要对浏览器、未知应用等非游戏场景生效
    rule_engine.AddRule(rule);
    
    // 规则10: 工作日 09:00-18:00，使用办公/写代码模式（优先级1）
    rule = Rule();
    rule.priority = 1;
    rule.target_mode = LightMode::WORK_CODING;
    Condition weekday_time_condition;
    weekday_time_condition.type = ConditionType::TIME_RANGE;
    weekday_time_condition.time_range = TimeRange(9, 0, 18, 0, WeekdayType::WEEKDAY);  // 工作日 09:00-18:00
    rule.conditions.push_back(weekday_time_condition);
    rule_engine.AddRule(rule);
    
    // 规则11: 周末 09:00-18:00，使用音乐律动模式（优先级0，作为娱乐模式）
    // 注意：这个优先级较低，会被其他规则（如游戏、视频等）覆盖
    rule = Rule();
    rule.priority = 0;
    rule.target_mode = LightMode::MUSIC;
    Condition weekend_time_condition;
    weekend_time_condition.type = ConditionType::TIME_RANGE;
    weekend_time_condition.time_range = TimeRange(9, 0, 18, 0, WeekdayType::WEEKEND);  // 周末 09:00-18:00
    rule.conditions.push_back(weekend_time_condition);
    rule_engine.AddRule(rule);
    
    // 注意：如果没有规则匹配，将返回默认模式（DEFAULT）
}

/**
 * 打印应用状态信息
 */
void PrintState(const WindowInfo& window_info, AppCategory category, CpuMonitor& cpu_monitor, 
                RuleEngine& rule_engine, const SystemState& system_state, bool debug_mode = false) {
    std::string timestamp = GetCurrentTimestamp();
    std::string current_time = GetCurrentTimeString();
    std::string weekday = GetWeekday();
    double cpu_usage = cpu_monitor.GetCpuUsage();
    double idle_minutes = GetUserIdleMinutes();
    
    // 决定灯光模式（对外接口：定期调用获取当前模式）
    LightMode light_mode = rule_engine.DecideLightMode(system_state);
    
    // Demo输出格式：清晰显示关键信息
    std::cout << "[" << timestamp << "]" << std::endl;
    std::cout << "  应用类别: " << AppClassifier::GetCategoryName(category) << std::endl;
    std::cout << "  当前时间: " << current_time << " (" << weekday << ")" << std::endl;
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "  CPU使用率: " << cpu_usage << "%" << std::endl;
    if (idle_minutes >= 0.0) {
        std::cout << "  Idle时间: " << idle_minutes << " 分钟" << std::endl;
    } else {
        std::cout << "  Idle时间: 无法获取" << std::endl;
    }
    std::cout << std::defaultfloat;
    std::cout << "  音频活动: " << (system_state.has_audio_activity ? "有" : "无") << std::endl;
    std::cout << "  当前判定的灯光模式: " << RuleEngine::GetLightModeName(light_mode) << std::endl;
    
    // 调试信息（可选）
    if (debug_mode) {
        std::cout << "  [调试] 进程名称: " << window_info.process_name << std::endl;
        std::cout << "  [调试] 窗口标题: " << window_info.window_title << std::endl;
        std::cout << "  [调试] 进程ID: " << window_info.process_id << std::endl;
    }
    
    // 调试模式：显示用于匹配的文本
    if (debug_mode) {
        std::string process_lower = window_info.process_name;
        std::string title_lower = window_info.window_title;
        std::transform(process_lower.begin(), process_lower.end(), process_lower.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        std::transform(title_lower.begin(), title_lower.end(), title_lower.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        std::cout << "  [调试] 匹配文本: " << process_lower << " " << title_lower << std::endl;
    }
    
    std::cout << std::string(60, '-') << std::endl;
}

/**
 * 主函数
 */
int main(int argc, char* argv[]) {
    // 设置控制台代码页为UTF-8以支持中文显示
    SetConsoleOutputCP(65001);
    
    // 设置信号处理
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
    
    // 解析命令行参数
    int interval_ms = 3000;  // 默认3秒
    bool debug_mode = false;  // 调试模式
    std::string config_file = "app_category_config.txt";  // 默认配置文件路径
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--debug" || arg == "-d") {
            debug_mode = true;
        } else if (arg == "--config" || arg == "-c") {
            // 指定配置文件路径
            if (i + 1 < argc) {
                config_file = argv[++i];
            } else {
                std::cerr << "错误: --config 参数需要指定文件路径" << std::endl;
            }
        } else {
            try {
                interval_ms = std::stoi(arg);
                if (interval_ms < 100) {
                    std::cerr << "警告: 间隔时间太短，将使用最小值100ms" << std::endl;
                    interval_ms = 100;
                }
            } catch (...) {
                // 忽略无效参数
            }
        }
    }
    
    std::cout << "应用状态监控程序" << std::endl;
    std::cout << "监控间隔: " << interval_ms << "ms" << std::endl;
    std::cout << "按 Ctrl+C 退出" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    WindowMonitor window_monitor;
    AppClassifier app_classifier;
    
    // 尝试从配置文件加载应用分类映射
    // 如果配置文件不存在，会使用默认映射
    if (!app_classifier.LoadConfigFile(config_file)) {
        std::cout << "提示: 未找到配置文件 \"" << config_file 
                  << "\"，使用默认应用分类映射" << std::endl;
    } else {
        std::cout << "已从配置文件加载应用分类: " << config_file << std::endl;
    }
    
    CpuMonitor cpu_monitor;
    AudioMonitor audio_monitor;
    RuleEngine rule_engine;
    
    // 初始化音频监控
    if (!audio_monitor.Initialize()) {
        std::cerr << "警告: 音频监控初始化失败，音频活动检测可能不可用" << std::endl;
    }
    
    // 初始化规则引擎，添加示例规则
    InitializeRules(rule_engine);
    
    // 用于跟踪上一次的灯光模式，检测变化
    LightMode last_light_mode = LightMode::DEFAULT;
    bool has_last_mode = false;
    
    while (g_running) {
        auto window_info_opt = window_monitor.GetForegroundWindowInfo();
        
        // 获取当前系统状态
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tm_buf;
        localtime_s(&tm_buf, &time_t);
        
        double cpu_usage = cpu_monitor.GetCpuUsage();
        double idle_minutes = GetUserIdleMinutes();
        bool has_audio = audio_monitor.GetAudioActivity();
        bool is_weekday = IsWeekday();
        
        SystemState system_state;
        system_state.current_hour = tm_buf.tm_hour;
        system_state.current_minute = tm_buf.tm_min;
        system_state.cpu_usage = cpu_usage;
        system_state.idle_minutes = idle_minutes >= 0.0 ? idle_minutes : 0.0;
        system_state.has_audio_activity = has_audio;
        system_state.is_weekday = is_weekday;
        
        // 决定当前灯光模式（对外接口：定期调用获取当前模式）
        LightMode current_light_mode = rule_engine.DecideLightMode(system_state);
        
        // 检测模式变化
        if (has_last_mode && last_light_mode != current_light_mode) {
            std::cout << std::endl;
            std::cout << ">>> Mode changed to: " << RuleEngine::GetLightModeName(current_light_mode) 
                      << " (from " << RuleEngine::GetLightModeName(last_light_mode) << ")" << std::endl;
            std::cout << std::endl;
        }
        
        if (window_info_opt.has_value()) {
            const WindowInfo& window_info = window_info_opt.value();
            
            // 分类应用
            AppCategory category = app_classifier.Classify(window_info);
            system_state.current_app_category = category;
            
            // 周期性输出完整状态信息（包括时间信息、CPU使用率和灯光模式）
            PrintState(window_info, category, cpu_monitor, rule_engine, system_state, debug_mode);
        } else {
            // 即使无法获取窗口信息，也显示时间信息、CPU使用率和用户空闲时间
            system_state.current_app_category = AppCategory::UNKNOWN;
            
            std::string current_time = GetCurrentTimeString();
            std::string weekday = GetWeekday();
            
            std::cout << "[" << GetCurrentTimestamp() << "] 无法获取窗口信息" << std::endl;
            std::cout << "  当前时间: " << current_time << std::endl;
            std::cout << "  星期: " << weekday << std::endl;
            std::cout << std::fixed << std::setprecision(1);
            std::cout << "  CPU使用率: " << cpu_usage << "%" << std::endl;
            if (idle_minutes >= 0.0) {
                std::cout << "  用户空闲时间: " << idle_minutes << " 分钟" << std::endl;
            } else {
                std::cout << "  用户空闲时间: 无法获取" << std::endl;
            }
            std::cout << std::defaultfloat;
            std::cout << "  音频活动: " << (system_state.has_audio_activity ? "有" : "无") << std::endl;
            std::cout << "  应用类别: 未知" << std::endl;
            std::cout << "  灯光模式: " << RuleEngine::GetLightModeName(current_light_mode) << std::endl;
            std::cout << std::string(60, '-') << std::endl;
        }
        
        // 更新上一次的模式
        last_light_mode = current_light_mode;
        has_last_mode = true;
        
        // 等待指定时间（3秒）
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
    }
    
    std::cout << std::endl << "程序已退出" << std::endl;
    return 0;
}


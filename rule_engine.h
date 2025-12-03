#pragma once

#include "app_classifier.h"
#include <string>
#include <vector>
#include <optional>
#include <ctime>
#include <cstdint>

/**
 * 灯光模式枚举
 */
enum class LightMode {
    GAME_SCREENSYNC,    // 游戏 / 屏幕同步
    VIDEO_CINEMATIC,    // 影视模式
    MUSIC,              // 音乐律动
    WORK_CODING,        // 办公 / 写代码静态模式
    NIGHT_DIM,          // 夜间弱光
    OFF,                // 关闭灯光
    DEFAULT             // 默认模式
};

/**
 * 条件类型枚举
 */
enum class ConditionType {
    APP_CATEGORY,       // 应用类别
    TIME_RANGE,         // 时间段
    CPU_THRESHOLD,      // CPU使用率阈值
    IDLE_THRESHOLD      // Idle时间阈值
};

/**
 * 时间段结构
 */
struct TimeRange {
    int start_hour;     // 开始小时 (0-23)
    int start_minute;   // 开始分钟 (0-59)
    int end_hour;       // 结束小时 (0-23)
    int end_minute;     // 结束分钟 (0-59)
    
    TimeRange(int sh = 0, int sm = 0, int eh = 23, int em = 59)
        : start_hour(sh), start_minute(sm), end_hour(eh), end_minute(em) {}
};

/**
 * 条件结构
 */
struct Condition {
    ConditionType type;
    
    // 根据类型使用不同的字段
    std::optional<AppCategory> app_category;      // APP_CATEGORY
    std::optional<TimeRange> time_range;           // TIME_RANGE
    std::optional<double> cpu_threshold;           // CPU_THRESHOLD (阈值)
    bool cpu_greater_than;                         // CPU_THRESHOLD (是否大于阈值)
    std::optional<double> idle_threshold;          // IDLE_THRESHOLD (阈值，分钟)
    bool idle_greater_than;                        // IDLE_THRESHOLD (是否大于等于阈值)
    
    Condition() : cpu_greater_than(false), idle_greater_than(false) {}
};

/**
 * 规则结构
 */
struct Rule {
    std::vector<Condition> conditions;    // 条件列表（AND逻辑）
    LightMode target_mode;                 // 目标灯光模式
    int priority;                          // 优先级（数值越大优先级越高）
    
    Rule() : target_mode(LightMode::DEFAULT), priority(0) {}
};

/**
 * 系统状态结构（用于规则匹配）
 */
struct SystemState {
    AppCategory current_app_category;    // 当前应用类别
    double cpu_usage;                      // CPU使用率 (0-100)
    double idle_minutes;                  // 用户空闲时间（分钟）
    int current_hour;                      // 当前小时 (0-23)
    int current_minute;                   // 当前分钟 (0-59)
};

/**
 * 规则引擎类
 * 负责规则管理和灯光模式决策
 */
class RuleEngine {
public:
    RuleEngine();
    
    /**
     * 添加规则
     * @param rule 规则对象
     */
    void AddRule(const Rule& rule);
    
    /**
     * 清空所有规则
     */
    void ClearRules();
    
    /**
     * 根据当前系统状态决定灯光模式
     * @param state 系统状态
     * @return 应该使用的灯光模式
     */
    LightMode DecideLightMode(const SystemState& state);
    
    /**
     * 获取灯光模式的中文名称
     * @param mode 灯光模式
     * @return 中文名称
     */
    static std::string GetLightModeName(LightMode mode);
    
    /**
     * 获取条件类型的中文名称
     * @param type 条件类型
     * @return 中文名称
     */
    static std::string GetConditionTypeName(ConditionType type);

private:
    std::vector<Rule> rules_;
    
    /**
     * 检查条件是否满足
     * @param condition 条件
     * @param state 系统状态
     * @return 是否满足
     */
    bool CheckCondition(const Condition& condition, const SystemState& state);
    
    /**
     * 检查时间段条件
     * @param time_range 时间段
     * @param current_hour 当前小时
     * @param current_minute 当前分钟
     * @return 是否在时间段内
     */
    bool CheckTimeRange(const TimeRange& time_range, int current_hour, int current_minute);
};


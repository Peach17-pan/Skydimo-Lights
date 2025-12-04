#include "rule_engine.h"
#include <algorithm>

RuleEngine::RuleEngine() {
    // 可以添加一些默认规则
}

void RuleEngine::AddRule(const Rule& rule) {
    rules_.push_back(rule);
    // 按优先级排序（优先级高的在前，便于查找）
    std::sort(rules_.begin(), rules_.end(), 
        [](const Rule& a, const Rule& b) {
            return a.priority > b.priority;
        });
}

void RuleEngine::ClearRules() {
    rules_.clear();
}
LightMode RuleEngine::DecideLightMode(const SystemState& state) {
    // 遍历所有规则，找到第一个所有条件都满足的规则
    // 由于已经按优先级排序，第一个匹配的就是优先级最高的
    for (const auto& rule : rules_) {
        bool all_conditions_met = true;
        
        // 检查所有条件（AND逻辑）
        for (const auto& condition : rule.conditions) {
            if (!CheckCondition(condition, state)) {
                all_conditions_met = false;
                break;
            }
        }
        
        // 如果所有条件都满足，返回该规则的目标模式
        if (all_conditions_met) {
            return rule.target_mode;
        }
    }
    
    // 没有规则匹配，返回默认模式
    return LightMode::DEFAULT;
}

bool RuleEngine::CheckCondition(const Condition& condition, const SystemState& state) {
    switch (condition.type) {
        case ConditionType::APP_CATEGORY:
            if (condition.app_category.has_value()) {
                return state.current_app_category == condition.app_category.value();
            }
            return false;
            
        case ConditionType::TIME_RANGE:
            if (condition.time_range.has_value()) {
                return CheckTimeRange(condition.time_range.value(), 
                                     state.current_hour, state.current_minute,
                                     state.is_weekday);
            }
            return false;
            
        case ConditionType::CPU_THRESHOLD:
            if (condition.cpu_threshold.has_value()) {
                if (condition.cpu_greater_than) {
                    return state.cpu_usage > condition.cpu_threshold.value();
                } else {
                    return state.cpu_usage <= condition.cpu_threshold.value();
                }
            }
            return false;
            
        case ConditionType::IDLE_THRESHOLD:
            if (condition.idle_threshold.has_value()) {
                if (condition.idle_greater_than) {
                    return state.idle_minutes >= condition.idle_threshold.value();
                } else {
                    return state.idle_minutes < condition.idle_threshold.value();
                }
            }
            return false;
            
        case ConditionType::AUDIO_ACTIVITY:
            if (condition.audio_activity.has_value()) {
                return state.has_audio_activity == condition.audio_activity.value();
            }
            return false;
            
        default:
            return false;
    }
}

bool RuleEngine::CheckTimeRange(const TimeRange& time_range, int current_hour, int current_minute, bool is_weekday) {
    // 检查工作日类型限制
    if (time_range.weekday_type == WeekdayType::WEEKDAY && !is_weekday) {
        return false;  // 要求工作日，但当前是周末
    }
    if (time_range.weekday_type == WeekdayType::WEEKEND && is_weekday) {
        return false;  // 要求周末，但当前是工作日
    }
    
    // 将时间转换为分钟数，便于比较
    int start_minutes = time_range.start_hour * 60 + time_range.start_minute;
    int end_minutes = time_range.end_hour * 60 + time_range.end_minute;
    int current_minutes = current_hour * 60 + current_minute;
    
    // 处理跨天的情况（如 23:00-07:00）
    if (start_minutes > end_minutes) {
        // 跨天：当前时间在开始时间之后或结束时间之前
        return current_minutes >= start_minutes || current_minutes <= end_minutes;
    } else {
        // 不跨天：当前时间在开始和结束时间之间
        return current_minutes >= start_minutes && current_minutes <= end_minutes;
    }
}

std::string RuleEngine::GetLightModeName(LightMode mode) {
    switch (mode) {
        case LightMode::GAME_SCREENSYNC:
            return "游戏/屏幕同步";
        case LightMode::VIDEO_CINEMATIC:
            return "影视模式";
        case LightMode::MUSIC:
            return "音乐律动";
        case LightMode::WORK_CODING:
            return "办公/写代码";
        case LightMode::NIGHT_DIM:
            return "夜间弱光";
        case LightMode::OFF:
            return "关闭灯光";
        case LightMode::DEFAULT:
        default:
            return "默认模式";
    }
}

std::string RuleEngine::GetConditionTypeName(ConditionType type) {
    switch (type) {
        case ConditionType::APP_CATEGORY:
            return "应用类别";
        case ConditionType::TIME_RANGE:
            return "时间段";
        case ConditionType::CPU_THRESHOLD:
            return "CPU使用率";
        case ConditionType::IDLE_THRESHOLD:
            return "空闲时间";
        case ConditionType::AUDIO_ACTIVITY:
            return "音频活动";
        default:
            return "未知";
    }
}


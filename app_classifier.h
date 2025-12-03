#pragma once

#include "window_monitor.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

/**
 * 应用类别枚举
 */
enum class AppCategory {
    GAME,           // 游戏类
    VIDEO,          // 电影/视频类
    MUSIC,          // 音乐类
    DOCUMENT,       // 文档/办公类
    BROWSER,        // 浏览器/上网类
    DEVELOPMENT,    // 开发/编程类
    CREATIVE,        // 创作类
    UNKNOWN         // 未知
};

/**
 * 应用分类器类
 * 根据进程名和窗口标题对应用进行分类
 */
class AppClassifier {
public:
    AppClassifier();
    
    /**
     * 对应用进行分类
     * @param window_info 窗口信息
     * @return AppCategory枚举值
     */
    AppCategory Classify(const WindowInfo& window_info);
    
    /**
     * 获取类别的中文名称
     * @param category 应用类别
     * @return 中文名称
     */
    static std::string GetCategoryName(AppCategory category);

private:
    std::unordered_set<std::string> game_keywords_;
    std::unordered_set<std::string> video_keywords_;
    std::unordered_set<std::string> music_keywords_;
    std::unordered_set<std::string> document_keywords_;
    std::unordered_set<std::string> browser_keywords_;
    std::unordered_set<std::string> development_keywords_;
    std::unordered_set<std::string> creative_keywords_;
    
    std::unordered_map<std::string, AppCategory> process_name_mapping_;
    
    /**
     * 初始化关键词集合
     */
    void InitializeKeywords();
    
    /**
     * 初始化进程名映射
     */
    void InitializeProcessNameMapping();
    
    /**
     * 检查文本中是否包含关键词集合中的任何关键词
     * @param text 要检查的文本
     * @param keywords 关键词集合
     * @return 如果包含任何关键词则返回true
     */
    bool ContainsKeywords(const std::string& text, const std::unordered_set<std::string>& keywords);
    
    /**
     * 将字符串转换为小写
     */
    std::string ToLower(const std::string& str);
};


#include "app_classifier.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

AppClassifier::AppClassifier() {
    InitializeKeywords();
    // 尝试从配置文件加载，如果失败则使用默认映射
    if (!InitializeProcessNameMapping("app_category_config.txt")) {
        // 配置文件不存在或加载失败，使用默认映射
        InitializeDefaultMapping();
    }
}

void AppClassifier::InitializeKeywords() {
    // 游戏类关键词
    game_keywords_ = {
        "steam", "epic", "origin", "battle.net", "riot", "valorant",
        "league of legends", "csgo", "counter-strike", "dota", "apex",
        "fortnite", "minecraft", "roblox", "unity", "unreal", "game",
        "gaming", "play", "launcher"
    };
    
    // 视频类关键词
    video_keywords_ = {
        "vlc", "potplayer", "mpc", "media player", "kodi", "plex",
        "netflix", "youtube", "bilibili", "youku", "iqiyi", "tencent video",
        "disney", "hbo", "prime video", "player", "播放器", "视频",
        "movie", "film", "media", "streaming"
    };
    
    // 音乐类关键词
    music_keywords_ = {
        "spotify", "music", "网易云音乐", "qq音乐", "酷狗", "酷我",
        "foobar", "winamp", "itunes", "apple music", "youtube music",
        "soundcloud", "musicbee", "aimp", "audacious", "音乐", "播放器"
    };
    
    // 文档/办公类关键词
    document_keywords_ = {
        "word", "excel", "powerpoint", "outlook", "onenote", "office",
        "wps", "libreoffice", "openoffice", "notepad", "notepad++",
        "wordpad", "pdf", "adobe reader", "foxit", "文档", "办公",
        "microsoft", "writer", "calc", "impress"
    };
    
    // 浏览器类关键词
    browser_keywords_ = {
        "chrome", "firefox", "edge", "safari", "opera", "brave",
        "vivaldi", "tor", "browser", "浏览器", "iexplore", "msedge"
    };
    
    // 开发/编程类关键词
    development_keywords_ = {
        "visual studio", "vscode", "vs code", "code.exe", "pycharm", "intellij", "eclipse",
        "android studio", "xcode", "sublime", "atom", "vim", "emacs",
        "github", "gitlab", "docker", "kubernetes", "terminal",
        "powershell", "bash", "zsh", "ide", "editor", "开发",
        "编程", "jetbrains", "rider", "clion", "cursor", "devenv"
    };
    
    // 创作类关键词
    creative_keywords_ = {
        "photoshop", "illustrator", "premiere", "after effects", "ae",
        "davinci", "resolve", "final cut", "blender", "maya", "3ds max",
        "cinema 4d", "sketch", "figma", "adobe", "creative", "创作",
        "剪辑", "设计", "ps", "ai", "pr", "c4d", "unity", "unreal"
    };
}

bool AppClassifier::InitializeProcessNameMapping(const std::string& config_file_path) {
    if (config_file_path.empty()) {
        return false;
    }
    
    return LoadFromConfigFile(config_file_path);
}

bool AppClassifier::LoadConfigFile(const std::string& config_file_path) {
    // 清空现有映射
    process_name_mapping_.clear();
    
    // 尝试从配置文件加载
    if (LoadFromConfigFile(config_file_path)) {
        return true;
    }
    
    // 如果加载失败，使用默认映射
    InitializeDefaultMapping();
    return false;
}

bool AppClassifier::LoadFromConfigFile(const std::string& config_file_path) {
    std::ifstream file(config_file_path);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    int line_number = 0;
    int loaded_count = 0;
    
    while (std::getline(file, line)) {
        line_number++;
        
        // 去除首尾空白字符
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        // 跳过空行和注释
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // 解析格式：进程名=类别名
        size_t equal_pos = line.find('=');
        if (equal_pos == std::string::npos) {
            // 格式错误，跳过这一行
            continue;
        }
        
        std::string process_name = line.substr(0, equal_pos);
        std::string category_name = line.substr(equal_pos + 1);
        
        // 去除空白字符
        process_name.erase(0, process_name.find_first_not_of(" \t"));
        process_name.erase(process_name.find_last_not_of(" \t") + 1);
        category_name.erase(0, category_name.find_first_not_of(" \t"));
        category_name.erase(category_name.find_last_not_of(" \t") + 1);
        
        if (process_name.empty() || category_name.empty()) {
            continue;
        }
        
        // 转换为小写
        process_name = ToLower(process_name);
        category_name = ToLower(category_name);
        
        // 解析类别名
        AppCategory category;
        if (category_name == "game") {
            category = AppCategory::GAME;
        } else if (category_name == "video") {
            category = AppCategory::VIDEO;
        } else if (category_name == "music") {
            category = AppCategory::MUSIC;
        } else if (category_name == "document") {
            category = AppCategory::DOCUMENT;
        } else if (category_name == "browser") {
            category = AppCategory::BROWSER;
        } else if (category_name == "development") {
            category = AppCategory::DEVELOPMENT;
        } else if (category_name == "creative") {
            category = AppCategory::CREATIVE;
        } else {
            // 未知类别，跳过
            continue;
        }
        
        // 添加到映射表
        process_name_mapping_[process_name] = category;
        loaded_count++;
    }
    
    file.close();
    return loaded_count > 0;
}

void AppClassifier::InitializeDefaultMapping() {
    // 游戏平台
    process_name_mapping_["steam.exe"] = AppCategory::GAME;
    process_name_mapping_["epicgameslauncher.exe"] = AppCategory::GAME;
    process_name_mapping_["origin.exe"] = AppCategory::GAME;
    process_name_mapping_["battle.net.exe"] = AppCategory::GAME;
    process_name_mapping_["riotclientservices.exe"] = AppCategory::GAME;
    
    // 视频播放器
    process_name_mapping_["vlc.exe"] = AppCategory::VIDEO;
    process_name_mapping_["potplayermini64.exe"] = AppCategory::VIDEO;
    process_name_mapping_["potplayermini.exe"] = AppCategory::VIDEO;
    process_name_mapping_["mpc-hc.exe"] = AppCategory::VIDEO;
    process_name_mapping_["kodi.exe"] = AppCategory::VIDEO;
    
    // 音乐播放器
    process_name_mapping_["spotify.exe"] = AppCategory::MUSIC;
    process_name_mapping_["musicbee.exe"] = AppCategory::MUSIC;
    process_name_mapping_["foobar2000.exe"] = AppCategory::MUSIC;
    process_name_mapping_["itunes.exe"] = AppCategory::MUSIC;
    
    // 办公软件
    process_name_mapping_["winword.exe"] = AppCategory::DOCUMENT;
    process_name_mapping_["excel.exe"] = AppCategory::DOCUMENT;
    process_name_mapping_["powerpnt.exe"] = AppCategory::DOCUMENT;
    process_name_mapping_["outlook.exe"] = AppCategory::DOCUMENT;
    process_name_mapping_["onenote.exe"] = AppCategory::DOCUMENT;
    process_name_mapping_["wps.exe"] = AppCategory::DOCUMENT;
    process_name_mapping_["notepad++.exe"] = AppCategory::DOCUMENT;
    process_name_mapping_["notepad.exe"] = AppCategory::DOCUMENT;
    
    // 浏览器
    process_name_mapping_["chrome.exe"] = AppCategory::BROWSER;
    process_name_mapping_["firefox.exe"] = AppCategory::BROWSER;
    process_name_mapping_["msedge.exe"] = AppCategory::BROWSER;
    process_name_mapping_["opera.exe"] = AppCategory::BROWSER;
    process_name_mapping_["brave.exe"] = AppCategory::BROWSER;
    process_name_mapping_["vivaldi.exe"] = AppCategory::BROWSER;
    process_name_mapping_["iexplore.exe"] = AppCategory::BROWSER;
    
    // 开发工具
    process_name_mapping_["devenv.exe"] = AppCategory::DEVELOPMENT;
    process_name_mapping_["code.exe"] = AppCategory::DEVELOPMENT;
    process_name_mapping_["pycharm64.exe"] = AppCategory::DEVELOPMENT;
    process_name_mapping_["pycharm.exe"] = AppCategory::DEVELOPMENT;
    process_name_mapping_["idea64.exe"] = AppCategory::DEVELOPMENT;
    process_name_mapping_["idea.exe"] = AppCategory::DEVELOPMENT;
    process_name_mapping_["eclipse.exe"] = AppCategory::DEVELOPMENT;
    process_name_mapping_["sublime_text.exe"] = AppCategory::DEVELOPMENT;
    process_name_mapping_["cursor.exe"] = AppCategory::DEVELOPMENT;
    process_name_mapping_["notepad++.exe"] = AppCategory::DEVELOPMENT;
    
    // 创作工具
    process_name_mapping_["photoshop.exe"] = AppCategory::CREATIVE;
    process_name_mapping_["illustrator.exe"] = AppCategory::CREATIVE;
    process_name_mapping_["premiere pro.exe"] = AppCategory::CREATIVE;
    process_name_mapping_["afterfx.exe"] = AppCategory::CREATIVE;
    process_name_mapping_["davinci resolve.exe"] = AppCategory::CREATIVE;
    process_name_mapping_["blender.exe"] = AppCategory::CREATIVE;
}

AppCategory AppClassifier::Classify(const WindowInfo& window_info) {
    // 提取纯进程名（去除路径，只保留文件名）
    std::string process_name = window_info.process_name;
    size_t last_slash = process_name.find_last_of("\\/");
    if (last_slash != std::string::npos) {
        process_name = process_name.substr(last_slash + 1);
    }
    
    std::string process_name_lower = ToLower(process_name);
    std::string window_title_lower = ToLower(window_info.window_title);
    
    // 首先检查精确的进程名映射
    auto it = process_name_mapping_.find(process_name_lower);
    if (it != process_name_mapping_.end()) {
        return it->second;
    }
    
    // 检查进程名和窗口标题中的关键词
    std::string combined_text = process_name_lower + " " + window_title_lower;
    
    // 按优先级检查各类别
    if (ContainsKeywords(combined_text, game_keywords_)) {
        return AppCategory::GAME;
    }
    
    if (ContainsKeywords(combined_text, video_keywords_)) {
        return AppCategory::VIDEO;
    }
    
    if (ContainsKeywords(combined_text, music_keywords_)) {
        return AppCategory::MUSIC;
    }
    
    if (ContainsKeywords(combined_text, browser_keywords_)) {
        return AppCategory::BROWSER;
    }
    
    if (ContainsKeywords(combined_text, development_keywords_)) {
        return AppCategory::DEVELOPMENT;
    }
    
    if (ContainsKeywords(combined_text, creative_keywords_)) {
        return AppCategory::CREATIVE;
    }
    
    if (ContainsKeywords(combined_text, document_keywords_)) {
        return AppCategory::DOCUMENT;
    }
    
    return AppCategory::UNKNOWN;
}

std::string AppClassifier::GetCategoryName(AppCategory category) {
    switch (category) {
        case AppCategory::GAME:
            return "游戏类";
        case AppCategory::VIDEO:
            return "电影/视频类";
        case AppCategory::MUSIC:
            return "音乐类";
        case AppCategory::DOCUMENT:
            return "文档/办公类";
        case AppCategory::BROWSER:
            return "浏览器/上网类";
        case AppCategory::DEVELOPMENT:
            return "开发/编程类";
        case AppCategory::CREATIVE:
            return "创作类";
        case AppCategory::UNKNOWN:
        default:
            return "未知";
    }
}

bool AppClassifier::ContainsKeywords(const std::string& text, const std::unordered_set<std::string>& keywords) {
    for (const auto& keyword : keywords) {
        if (text.find(keyword) != std::string::npos) {
            return true;
        }
    }
    return false;
}

std::string AppClassifier::ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}


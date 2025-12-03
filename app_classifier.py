"""
应用分类器模块 - 根据应用信息进行分类
"""
from typing import Dict, Set
from enum import Enum
from window_monitor import WindowInfo


class AppCategory(Enum):
    """应用类别枚举"""
    GAME = "游戏类"
    VIDEO = "电影/视频类"
    MUSIC = "音乐类"
    DOCUMENT = "文档/办公类"
    BROWSER = "浏览器/上网类"
    DEVELOPMENT = "开发/编程类"
    CREATIVE = "创作类"
    UNKNOWN = "未知"


class AppClassifier:
    """应用分类器 - 根据进程名和窗口标题对应用进行分类"""
    
    def __init__(self):
        # 游戏类关键词
        self._game_keywords: Set[str] = {
            'steam', 'epic', 'origin', 'battle.net', 'riot', 'valorant',
            'league of legends', 'csgo', 'counter-strike', 'dota', 'apex',
            'fortnite', 'minecraft', 'roblox', 'unity', 'unreal', 'game',
            'gaming', 'play', 'launcher'
        }
        
        # 视频类关键词
        self._video_keywords: Set[str] = {
            'vlc', 'potplayer', 'mpc', 'media player', 'kodi', 'plex',
            'netflix', 'youtube', 'bilibili', 'youku', 'iqiyi', 'tencent video',
            'disney', 'hbo', 'prime video', 'player', '播放器', '视频',
            'movie', 'film', 'media', 'streaming'
        }
        
        # 音乐类关键词
        self._music_keywords: Set[str] = {
            'spotify', 'music', '网易云音乐', 'qq音乐', '酷狗', '酷我',
            'foobar', 'winamp', 'itunes', 'apple music', 'youtube music',
            'soundcloud', 'musicbee', 'aimp', 'audacious', '音乐', '播放器'
        }
        
        # 文档/办公类关键词
        self._document_keywords: Set[str] = {
            'word', 'excel', 'powerpoint', 'outlook', 'onenote', 'office',
            'wps', 'libreoffice', 'openoffice', 'notepad', 'notepad++',
            'wordpad', 'pdf', 'adobe reader', 'foxit', '文档', '办公',
            'microsoft', 'office', 'writer', 'calc', 'impress'
        }
        
        # 浏览器类关键词
        self._browser_keywords: Set[str] = {
            'chrome', 'firefox', 'edge', 'safari', 'opera', 'brave',
            'vivaldi', 'tor', 'browser', '浏览器', 'iexplore', 'msedge'
        }
        
        # 开发/编程类关键词
        self._development_keywords: Set[str] = {
            'visual studio', 'code', 'pycharm', 'intellij', 'eclipse',
            'android studio', 'xcode', 'sublime', 'atom', 'vim', 'emacs',
            'git', 'github', 'gitlab', 'docker', 'kubernetes', 'terminal',
            'cmd', 'powershell', 'bash', 'zsh', 'ide', 'editor', '开发',
            '编程', 'vs code', 'vscode', 'jetbrains', 'rider', 'clion'
        }
        
        # 创作类关键词
        self._creative_keywords: Set[str] = {
            'photoshop', 'illustrator', 'premiere', 'after effects', 'ae',
            'davinci', 'resolve', 'final cut', 'blender', 'maya', '3ds max',
            'cinema 4d', 'sketch', 'figma', 'adobe', 'creative', '创作',
            '剪辑', '设计', 'ps', 'ai', 'pr', 'ae', 'c4d', 'unity', 'unreal'
        }
        
        # 进程名到类别的直接映射（更精确）
        self._process_name_mapping: Dict[str, AppCategory] = {
            # 游戏平台
            'steam.exe': AppCategory.GAME,
            'epicgameslauncher.exe': AppCategory.GAME,
            'origin.exe': AppCategory.GAME,
            'battle.net.exe': AppCategory.GAME,
            'riotclientservices.exe': AppCategory.GAME,
            
            # 视频播放器
            'vlc.exe': AppCategory.VIDEO,
            'potplayermini64.exe': AppCategory.VIDEO,
            'potplayermini.exe': AppCategory.VIDEO,
            'mpc-hc.exe': AppCategory.VIDEO,
            'kodi.exe': AppCategory.VIDEO,
            
            # 音乐播放器
            'spotify.exe': AppCategory.MUSIC,
            'musicbee.exe': AppCategory.MUSIC,
            'foobar2000.exe': AppCategory.MUSIC,
            'itunes.exe': AppCategory.MUSIC,
            
            # 办公软件
            'winword.exe': AppCategory.DOCUMENT,
            'excel.exe': AppCategory.DOCUMENT,
            'powerpnt.exe': AppCategory.DOCUMENT,
            'outlook.exe': AppCategory.DOCUMENT,
            'onenote.exe': AppCategory.DOCUMENT,
            'wps.exe': AppCategory.DOCUMENT,
            'notepad++.exe': AppCategory.DOCUMENT,
            'notepad.exe': AppCategory.DOCUMENT,
            
            # 浏览器
            'chrome.exe': AppCategory.BROWSER,
            'firefox.exe': AppCategory.BROWSER,
            'msedge.exe': AppCategory.BROWSER,
            'opera.exe': AppCategory.BROWSER,
            'brave.exe': AppCategory.BROWSER,
            
            # 开发工具
            'devenv.exe': AppCategory.DEVELOPMENT,
            'code.exe': AppCategory.DEVELOPMENT,
            'pycharm64.exe': AppCategory.DEVELOPMENT,
            'idea64.exe': AppCategory.DEVELOPMENT,
            'eclipse.exe': AppCategory.DEVELOPMENT,
            'sublime_text.exe': AppCategory.DEVELOPMENT,
            
            # 创作工具
            'photoshop.exe': AppCategory.CREATIVE,
            'illustrator.exe': AppCategory.CREATIVE,
            'premiere pro.exe': AppCategory.CREATIVE,
            'afterfx.exe': AppCategory.CREATIVE,
            'davinci resolve.exe': AppCategory.CREATIVE,
            'blender.exe': AppCategory.CREATIVE,
        }
    
    def classify(self, window_info: WindowInfo) -> AppCategory:
        """
        对应用进行分类
        
        Args:
            window_info: 窗口信息对象
            
        Returns:
            AppCategory枚举值
        """
        if not window_info:
            return AppCategory.UNKNOWN
        
        process_name_lower = window_info.process_name.lower()
        window_title_lower = window_info.window_title.lower()
        
        # 首先检查精确的进程名映射
        if process_name_lower in self._process_name_mapping:
            return self._process_name_mapping[process_name_lower]
        
        # 检查进程名和窗口标题中的关键词
        combined_text = f"{process_name_lower} {window_title_lower}"
        
        # 按优先级检查各类别
        if self._contains_keywords(combined_text, self._game_keywords):
            return AppCategory.GAME
        
        if self._contains_keywords(combined_text, self._video_keywords):
            return AppCategory.VIDEO
        
        if self._contains_keywords(combined_text, self._music_keywords):
            return AppCategory.MUSIC
        
        if self._contains_keywords(combined_text, self._browser_keywords):
            return AppCategory.BROWSER
        
        if self._contains_keywords(combined_text, self._development_keywords):
            return AppCategory.DEVELOPMENT
        
        if self._contains_keywords(combined_text, self._creative_keywords):
            return AppCategory.CREATIVE
        
        if self._contains_keywords(combined_text, self._document_keywords):
            return AppCategory.DOCUMENT
        
        return AppCategory.UNKNOWN
    
    def _contains_keywords(self, text: str, keywords: Set[str]) -> bool:
        """
        检查文本中是否包含关键词集合中的任何关键词
        
        Args:
            text: 要检查的文本
            keywords: 关键词集合
            
        Returns:
            如果包含任何关键词则返回True
        """
        return any(keyword in text for keyword in keywords)


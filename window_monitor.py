"""
窗口监控模块 - 获取前台窗口信息
"""
import win32gui
import win32process
import win32api
import win32con
import psutil
from typing import Optional, Dict, Tuple
from dataclasses import dataclass


@dataclass
class WindowInfo:
    """窗口信息数据类"""
    process_name: str  # 可执行文件名
    window_title: str  # 窗口标题
    is_near_fullscreen: bool  # 是否接近全屏
    process_id: int  # 进程ID
    executable_path: Optional[str] = None  # 可执行文件路径


class WindowMonitor:
    """窗口监控器 - 负责获取前台窗口信息"""
    
    def __init__(self):
        self._screen_width = win32api.GetSystemMetrics(win32con.SM_CXSCREEN)
        self._screen_height = win32api.GetSystemMetrics(win32con.SM_CYSCREEN)
        self._fullscreen_threshold = 0.95  # 95%以上视为接近全屏
    
    def get_foreground_window_info(self) -> Optional[WindowInfo]:
        """
        获取当前前台窗口信息
        
        Returns:
            WindowInfo对象，如果获取失败则返回None
        """
        try:
            # 获取前台窗口句柄
            hwnd = win32gui.GetForegroundWindow()
            if not hwnd:
                return None
            
            # 获取窗口标题
            window_title = win32gui.GetWindowText(hwnd)
            
            # 获取进程ID
            _, process_id = win32process.GetWindowThreadProcessId(hwnd)
            
            # 获取进程信息
            process_name = None
            executable_path = None
            try:
                process = psutil.Process(process_id)
                process_name = process.name()
                try:
                    executable_path = process.exe()
                except (psutil.AccessDenied, psutil.NoSuchProcess):
                    executable_path = None
            except (psutil.NoSuchProcess, psutil.AccessDenied):
                # 如果无法获取进程信息，使用进程ID作为后备
                process_name = f"Process_{process_id}"
                executable_path = None
            
            # 检查是否接近全屏
            is_near_fullscreen = self._check_near_fullscreen(hwnd)
            
            return WindowInfo(
                process_name=process_name,
                window_title=window_title,
                is_near_fullscreen=is_near_fullscreen,
                process_id=process_id,
                executable_path=executable_path
            )
        except Exception as e:
            print(f"获取窗口信息时出错: {e}")
            return None
    
    def _check_near_fullscreen(self, hwnd: int) -> bool:
        """
        检查窗口是否接近全屏
        
        Args:
            hwnd: 窗口句柄
            
        Returns:
            如果窗口占据屏幕95%以上则返回True
        """
        try:
            # 获取窗口矩形
            rect = win32gui.GetWindowRect(hwnd)
            left, top, right, bottom = rect
            
            # 计算窗口大小
            window_width = right - left
            window_height = bottom - top
            
            # 计算窗口占屏幕的比例
            width_ratio = window_width / self._screen_width
            height_ratio = window_height / self._screen_height
            
            # 如果宽度和高度都超过阈值，则认为接近全屏
            return width_ratio >= self._fullscreen_threshold and height_ratio >= self._fullscreen_threshold
        except Exception:
            return False


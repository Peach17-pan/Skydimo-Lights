"""
主程序入口 - 周期性获取前台应用状态
"""
import time
import json
from datetime import datetime
from typing import Optional
from window_monitor import WindowMonitor, WindowInfo
from app_classifier import AppClassifier, AppCategory


class AppStateMonitor:
    """应用状态监控器 - 周期性获取并输出前台应用状态"""
    
    def __init__(self, interval: float = 1.0):
        """
        初始化监控器
        
        Args:
            interval: 监控间隔（秒），默认1秒
        """
        self.interval = interval
        self.window_monitor = WindowMonitor()
        self.app_classifier = AppClassifier()
        self.running = False
        self.last_window_info: Optional[WindowInfo] = None
        self.last_category: Optional[AppCategory] = None
    
    def start(self):
        """开始监控"""
        self.running = True
        print("开始监控前台应用状态...")
        print("按 Ctrl+C 停止监控\n")
        
        try:
            while self.running:
                self._monitor_once()
                time.sleep(self.interval)
        except KeyboardInterrupt:
            print("\n\n监控已停止")
            self.running = False
    
    def _monitor_once(self):
        """执行一次监控"""
        window_info = self.window_monitor.get_foreground_window_info()
        
        if not window_info:
            return
        
        # 如果窗口信息没有变化，跳过输出（可选）
        if (self.last_window_info and 
            self.last_window_info.process_id == window_info.process_id and
            self.last_window_info.window_title == window_info.window_title):
            return
        
        # 分类应用
        category = self.app_classifier.classify(window_info)
        
        # 输出状态信息
        self._print_state(window_info, category)
        
        # 保存当前状态
        self.last_window_info = window_info
        self.last_category = category
    
    def _print_state(self, window_info: WindowInfo, category: AppCategory):
        """
        打印应用状态信息
        
        Args:
            window_info: 窗口信息
            category: 应用类别
        """
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        
        print(f"[{timestamp}]")
        print(f"  进程名称: {window_info.process_name}")
        print(f"  窗口标题: {window_info.window_title}")
        print(f"  进程ID: {window_info.process_id}")
        if window_info.executable_path:
            print(f"  可执行路径: {window_info.executable_path}")
        print(f"  接近全屏: {'是' if window_info.is_near_fullscreen else '否'}")
        print(f"  应用类别: {category.value}")
        print("-" * 60)
    
    def get_current_state(self) -> Optional[dict]:
        """
        获取当前应用状态（字典格式）
        
        Returns:
            包含应用状态的字典，如果获取失败则返回None
        """
        window_info = self.window_monitor.get_foreground_window_info()
        if not window_info:
            return None
        
        category = self.app_classifier.classify(window_info)
        
        return {
            "timestamp": datetime.now().isoformat(),
            "process_name": window_info.process_name,
            "window_title": window_info.window_title,
            "process_id": window_info.process_id,
            "executable_path": window_info.executable_path,
            "is_near_fullscreen": window_info.is_near_fullscreen,
            "category": category.value
        }
    
    def get_current_state_json(self) -> Optional[str]:
        """
        获取当前应用状态（JSON格式）
        
        Returns:
            JSON字符串，如果获取失败则返回None
        """
        state = self.get_current_state()
        if not state:
            return None
        return json.dumps(state, ensure_ascii=False, indent=2)


def main():
    """主函数"""
    import argparse
    
    parser = argparse.ArgumentParser(description="监控前台应用状态")
    parser.add_argument(
        "-i", "--interval",
        type=float,
        default=1.0,
        help="监控间隔（秒），默认1.0秒"
    )
    parser.add_argument(
        "-o", "--output",
        choices=["console", "json"],
        default="console",
        help="输出格式，默认console"
    )
    
    args = parser.parse_args()
    
    monitor = AppStateMonitor(interval=args.interval)
    
    if args.output == "json":
        # JSON输出模式
        try:
            while True:
                state_json = monitor.get_current_state_json()
                if state_json:
                    print(state_json)
                time.sleep(args.interval)
        except KeyboardInterrupt:
            pass
    else:
        # 控制台输出模式
        monitor.start()


if __name__ == "__main__":
    main()


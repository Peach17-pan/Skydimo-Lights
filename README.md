# Skydimo Lights - 前台应用状态监控

周期性获取电脑当前前台应用状态，包括应用名称、窗口标题、全屏状态和应用分类。

## 功能特性

- ✅ 获取当前前台应用名称（可执行文件名）
- ✅ 获取窗口标题
- ✅ 检测是否接近全屏（95%以上）
- ✅ 自动分类应用为以下类别之一：
  - 游戏类
  - 电影/视频类
  - 音乐类
  - 文档/办公类
  - 浏览器/上网类
  - 开发/编程类
  - 创作类（PS/剪辑等）
  - 未知

## 安装依赖

```powershell
pip install -r requirements.txt
```

## 使用方法

### 基本使用（控制台输出）

```powershell
python main.py
```

### 自定义监控间隔

```powershell
python main.py -i 2.0  # 每2秒监控一次
```

### JSON格式输出

```powershell
python main.py -o json
```

### 组合使用

```powershell
python main.py -i 0.5 -o json  # 每0.5秒输出一次JSON格式
```

## 输出示例

### 控制台输出

```
[2024-01-15 14:30:25]
  进程名称: chrome.exe
  窗口标题: GitHub - Skydimo-Lights
  进程ID: 12345
  可执行路径: C:\Program Files\Google\Chrome\Application\chrome.exe
  接近全屏: 否
  应用类别: 浏览器/上网类
------------------------------------------------------------
```

### JSON输出

```json
{
  "timestamp": "2024-01-15T14:30:25.123456",
  "process_name": "chrome.exe",
  "window_title": "GitHub - Skydimo-Lights",
  "process_id": 12345,
  "executable_path": "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe",
  "is_near_fullscreen": false,
  "category": "浏览器/上网类"
}
```

## 项目结构

```
Skydimo-Lights/
├── main.py              # 主程序入口
├── window_monitor.py    # 窗口监控模块
├── app_classifier.py    # 应用分类器模块
├── requirements.txt     # 项目依赖
└── README.md           # 说明文档
```

## 技术实现

- **window_monitor.py**: 使用 `pywin32` 和 `psutil` 获取Windows前台窗口信息
- **app_classifier.py**: 基于关键词匹配和进程名映射的应用分类逻辑
- **main.py**: 周期性监控和状态输出

## 注意事项

- 仅支持 Windows 系统
- 需要管理员权限才能获取某些系统进程的详细信息
- 监控间隔建议不小于0.1秒，避免系统资源占用过高


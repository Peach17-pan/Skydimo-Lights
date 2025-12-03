# C++ 应用状态监控程序

这是一个用C++编写的Windows应用状态监控程序，可以周期性地获取当前前台应用的状态信息并进行自动分类。

## 功能特性

- ✅ 获取当前前台应用的进程名称（可执行文件名）
- ✅ 获取窗口标题
- ✅ 检测窗口是否接近全屏（95%以上）
- ✅ 自动将应用归类为以下类别之一：
  - 游戏类
  - 电影/视频类
  - 音乐类
  - 文档/办公类
  - 浏览器/上网类
  - 开发/编程类
  - 创作类（PS/剪辑等）
  - 未知

## 项目结构

```
.
├── main.cpp              # 主程序入口
├── window_monitor.h      # 窗口监控类头文件
├── window_monitor.cpp    # 窗口监控类实现
├── app_classifier.h      # 应用分类器头文件
├── app_classifier.cpp    # 应用分类器实现
├── CMakeLists.txt        # CMake构建配置
└── BUILD.md              # 详细编译说明
```

## 快速开始

### 编译

使用CMake编译（推荐）：

```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### 运行

```powershell
.\bin\Release\app_state_monitor.exe
```

或指定监控间隔（毫秒）：

```powershell
.\bin\Release\app_state_monitor.exe 500
```

## 技术实现

### 核心组件

1. **WindowMonitor** (`window_monitor.h/cpp`)
   - 使用Windows API获取前台窗口信息
   - 检测窗口是否全屏
   - 获取进程信息和可执行文件路径

2. **AppClassifier** (`app_classifier.h/cpp`)
   - 基于关键词和进程名映射进行分类
   - 支持中英文关键词匹配
   - 优先级分类机制

3. **主程序** (`main.cpp`)
   - 周期性监控循环
   - 状态变化检测（避免重复输出）
   - 优雅退出处理

### 使用的Windows API

- `GetForegroundWindow()` - 获取前台窗口句柄
- `GetWindowText()` - 获取窗口标题
- `GetWindowThreadProcessId()` - 获取进程ID
- `GetWindowRect()` - 获取窗口矩形
- `GetSystemMetrics()` - 获取屏幕尺寸
- `QueryFullProcessImageName()` - 获取进程可执行文件路径
- `CreateToolhelp32Snapshot()` - 获取进程信息（备用方法）

## 设计原则

- **SOLID原则**: 单一职责、开闭原则
- **DRY原则**: 避免代码重复
- **高内聚、低耦合**: 模块化设计，UI与核心逻辑分离

## 注意事项

1. 需要Windows 10或更高版本
2. 某些系统进程可能无法获取完整信息
3. 程序需要足够的权限来访问其他进程
4. 控制台需要支持UTF-8编码以正确显示中文

## 许可证

本项目与主项目使用相同的许可证。


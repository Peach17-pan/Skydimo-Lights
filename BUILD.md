# C++ 应用状态监控程序 - 编译说明

## 系统要求

- Windows 10 或更高版本
- CMake 3.10 或更高版本
- 支持C++17的编译器（Visual Studio 2017+ 或 MinGW-w64）

## 编译方法

### 方法1: 使用 Visual Studio

1. 打开 PowerShell 或命令提示符，进入项目目录

2. 创建构建目录并生成 Visual Studio 项目文件：
```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
```

3. 编译项目：
```powershell
cmake --build . --config Release
```

4. 运行程序：
```powershell
.\bin\Release\app_state_monitor.exe
```

### 方法2: 使用 MinGW-w64

1. 确保已安装 MinGW-w64 和 CMake

2. 创建构建目录并生成 Makefile：
```powershell
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
```

3. 编译项目：
```powershell
cmake --build .
```

4. 运行程序：
```powershell
.\bin\app_state_monitor.exe
```

### 方法3: 直接使用编译器（不使用CMake）

#### 使用 MSVC (Visual Studio)

```powershell
cl /EHsc /std:c++17 /utf-8 /W4 /O2 main.cpp window_monitor.cpp app_classifier.cpp /link psapi.lib /OUT:app_state_monitor.exe
```

#### 使用 MinGW-w64

```powershell
g++ -std=c++17 -Wall -O2 main.cpp window_monitor.cpp app_classifier.cpp -lpsapi -o app_state_monitor.exe
```

## 使用方法

### 基本用法

```powershell
.\app_state_monitor.exe
```

默认监控间隔为 1000ms (1秒)

### 自定义监控间隔

```powershell
.\app_state_monitor.exe 500
```

参数为监控间隔（毫秒），最小值为 100ms

### 退出程序

按 `Ctrl+C` 退出程序

## 输出示例

```
[2024-01-15 14:30:25.123]
  进程名称: chrome.exe
  窗口标题: GitHub - 正在使用 Cursor
  进程ID: 12345
  可执行路径: C:\Program Files\Google\Chrome\Application\chrome.exe
  接近全屏: 是
  应用类别: 浏览器/上网类
------------------------------------------------------------
```

## 功能说明

程序会周期性地获取当前前台应用的状态信息：

1. **进程名称**: 可执行文件名（如 chrome.exe）
2. **窗口标题**: 当前窗口的标题文本
3. **进程ID**: Windows进程ID
4. **可执行路径**: 可执行文件的完整路径（如果可获取）
5. **接近全屏**: 窗口是否占据屏幕95%以上
6. **应用类别**: 自动分类为以下类别之一：
   - 游戏类
   - 电影/视频类
   - 音乐类
   - 文档/办公类
   - 浏览器/上网类
   - 开发/编程类
   - 创作类
   - 未知

## 注意事项

1. 程序需要足够的权限来访问其他进程的信息
2. 某些系统进程可能无法获取完整信息
3. 如果窗口信息没有变化，程序会跳过输出（避免重复信息）
4. 控制台需要支持UTF-8编码以正确显示中文


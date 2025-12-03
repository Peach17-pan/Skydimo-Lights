# 快速测试指南

## 编译程序

```powershell
mkdir build -ErrorAction SilentlyContinue
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
cd ..
```

## 运行测试

### 方法1: 基本测试（默认1秒间隔）

```powershell
.\build\bin\Release\app_state_monitor.exe
```

### 方法2: 调试模式（推荐，可以看到匹配文本）

```powershell
.\build\bin\Release\app_state_monitor.exe --debug 500
```

### 方法3: 快速测试（500ms间隔）

```powershell
.\build\bin\Release\app_state_monitor.exe 500
```

## 测试步骤

1. **启动程序**后，程序会开始监控
2. **切换到不同的应用程序**，观察输出：
   - Chrome/Firefox/Edge → 应该显示"浏览器/上网类"
   - VS Code/Cursor → 应该显示"开发/编程类"  
   - Word/Excel → 应该显示"文档/办公类"
   - Steam → 应该显示"游戏类"
   - VLC/PotPlayer → 应该显示"电影/视频类"
3. **按 Ctrl+C 退出程序**

## 预期输出示例

```
应用状态监控程序
监控间隔: 500ms
按 Ctrl+C 退出
============================================================
[2024-01-15 14:30:25.123]
  进程名称: chrome.exe
  窗口标题: GitHub
  进程ID: 12345
  可执行路径: C:\Program Files\Google\Chrome\Application\chrome.exe
  接近全屏: 否
  应用类别: 浏览器/上网类
  [调试] 匹配文本: chrome.exe github
------------------------------------------------------------
```

## 如果分类显示"未知"

1. **使用调试模式** (`--debug`) 查看匹配文本
2. **检查进程名**是否正确（应该是纯文件名，如 `chrome.exe`）
3. **检查窗口标题**是否包含可识别的关键词
4. 如果发现问题，可以：
   - 在 `app_classifier.cpp` 中添加更多关键词
   - 在 `InitializeProcessNameMapping()` 中添加进程名映射

## 常见问题

**Q: 程序无法编译？**
A: 确保安装了 Visual Studio 2017+ 和 CMake 3.10+

**Q: 程序运行但没有输出？**
A: 尝试切换到不同的应用程序，程序只在窗口变化时输出

**Q: 分类一直显示"未知"？**
A: 使用 `--debug` 模式查看匹配文本，检查进程名和窗口标题


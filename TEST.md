# C++ 应用状态监控程序 - 功能验证指南

## 快速验证步骤

### 1. 编译程序

#### 方法A: 使用 Visual Studio（推荐）

```powershell
# 创建构建目录
mkdir build
cd build

# 生成 Visual Studio 项目文件
cmake .. -G "Visual Studio 17 2022" -A x64

# 编译 Release 版本
cmake --build . --config Release

# 返回项目根目录
cd ..
```

#### 方法B: 使用 MinGW-w64

```powershell
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
cd ..
```

#### 方法C: 直接使用编译器

**MSVC:**
```powershell
cl /EHsc /std:c++17 /utf-8 /W4 /O2 main.cpp window_monitor.cpp app_classifier.cpp /link psapi.lib /OUT:app_state_monitor.exe
```

**MinGW:**
```powershell
g++ -std=c++17 -Wall -O2 main.cpp window_monitor.cpp app_classifier.cpp -lpsapi -o app_state_monitor.exe
```

### 2. 运行程序

```powershell
# 使用默认间隔（1000ms = 1秒）
.\build\bin\Release\app_state_monitor.exe

# 或指定自定义间隔（500ms）
.\build\bin\Release\app_state_monitor.exe 500
```

### 3. 功能验证测试

#### 测试1: 基本功能验证

1. **启动程序**，应该看到：
   ```
   应用状态监控程序
   监控间隔: 1000ms
   按 Ctrl+C 退出
   ============================================================
   ```

2. **切换到不同的应用程序**，程序应该自动检测并输出：
   - 进程名称
   - 窗口标题
   - 进程ID
   - 可执行路径（如果可获取）
   - 是否接近全屏
   - 应用类别

#### 测试2: 分类功能验证

测试不同类别的应用，验证分类是否正确：

**浏览器类测试：**
- 打开 Chrome/Firefox/Edge
- 预期输出：`应用类别: 浏览器/上网类`

**开发类测试：**
- 打开 Visual Studio Code / Visual Studio
- 预期输出：`应用类别: 开发/编程类`

**游戏类测试：**
- 打开 Steam / Epic Games Launcher
- 预期输出：`应用类别: 游戏类`

**文档类测试：**
- 打开 Word / Excel / Notepad++
- 预期输出：`应用类别: 文档/办公类`

**视频类测试：**
- 打开 VLC / PotPlayer / YouTube（浏览器中）
- 预期输出：`应用类别: 电影/视频类`

**音乐类测试：**
- 打开 Spotify / 网易云音乐
- 预期输出：`应用类别: 音乐类`

**创作类测试：**
- 打开 Photoshop / Premiere / Blender
- 预期输出：`应用类别: 创作类`

#### 测试3: 全屏检测验证

1. **打开一个应用程序**（如浏览器）
2. **按 F11 进入全屏模式**，或**最大化窗口**
3. 验证输出中 `接近全屏: 是` 应该显示为 `是`
4. **退出全屏**，验证输出应该显示为 `否`

#### 测试4: 状态变化检测

1. **启动程序**
2. **保持当前窗口不变**，程序应该只输出一次
3. **切换到其他窗口**，程序应该立即输出新窗口信息
4. **切换回原窗口**，程序应该再次输出

#### 测试5: 监控间隔测试

```powershell
# 使用较短的间隔（500ms）
.\build\bin\Release\app_state_monitor.exe 500

# 快速切换窗口，验证输出频率
```

#### 测试6: 异常情况处理

1. **关闭所有窗口**（只保留桌面），程序应该继续运行
2. **切换到系统进程**（如任务管理器），验证是否能正确获取信息
3. **按 Ctrl+C**，验证程序是否能优雅退出

### 4. 预期输出示例

#### 示例1: 浏览器窗口

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

#### 示例2: 开发工具

```
[2024-01-15 14:30:26.456]
  进程名称: code.exe
  窗口标题: Visual Studio Code
  进程ID: 67890
  可执行路径: C:\Users\...\AppData\Local\Programs\Microsoft VS Code\Code.exe
  接近全屏: 否
  应用类别: 开发/编程类
------------------------------------------------------------
```

#### 示例3: 游戏

```
[2024-01-15 14:30:27.789]
  进程名称: steam.exe
  窗口标题: Steam
  进程ID: 11111
  可执行路径: C:\Program Files (x86)\Steam\steam.exe
  接近全屏: 否
  应用类别: 游戏类
------------------------------------------------------------
```

### 5. 常见问题排查

#### 问题1: 编译错误

**错误**: `无法找到 psapi.lib`
- **解决**: 确保使用正确的链接器选项，psapi.lib 是 Windows SDK 的一部分

**错误**: `C++17 特性不支持`
- **解决**: 确保编译器支持 C++17（Visual Studio 2017+ 或 GCC 7+）

#### 问题2: 运行时错误

**问题**: 无法获取某些进程信息
- **原因**: 某些系统进程需要管理员权限
- **解决**: 以管理员身份运行程序（如果需要）

**问题**: 中文显示乱码
- **解决**: 确保控制台支持 UTF-8，程序已设置 `SetConsoleOutputCP(65001)`

#### 问题3: 分类不准确

**问题**: 某些应用分类错误
- **解决**: 可以在 `app_classifier.cpp` 中添加更多关键词或进程名映射

### 6. 性能验证

程序应该：
- ✅ 占用 CPU 资源很少（< 1%）
- ✅ 内存占用小（< 10MB）
- ✅ 响应及时（窗口切换后立即检测）
- ✅ 无内存泄漏（长时间运行稳定）

### 7. 自动化测试建议

可以创建一个简单的测试脚本：

```powershell
# test.ps1
Write-Host "开始功能测试..." -ForegroundColor Green

# 编译
Write-Host "1. 编译程序..." -ForegroundColor Cyan
mkdir build -ErrorAction SilentlyContinue
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
cd ..

# 运行（5秒后自动退出）
Write-Host "2. 运行程序（5秒测试）..." -ForegroundColor Cyan
$job = Start-Job -ScriptBlock {
    Set-Location $using:PWD
    Start-Process -FilePath ".\build\bin\Release\app_state_monitor.exe" -ArgumentList "500" -NoNewWindow -Wait
}
Start-Sleep -Seconds 5
Stop-Process -Name "app_state_monitor" -ErrorAction SilentlyContinue
Stop-Job $job -ErrorAction SilentlyContinue

Write-Host "测试完成！" -ForegroundColor Green
```

### 8. 验证清单

- [ ] 程序能够成功编译
- [ ] 程序能够正常启动
- [ ] 能够正确获取进程名称
- [ ] 能够正确获取窗口标题
- [ ] 能够正确检测全屏状态
- [ ] 能够正确分类各种应用
- [ ] 状态变化时能及时输出
- [ ] 能够优雅退出（Ctrl+C）
- [ ] 中文显示正常
- [ ] 长时间运行稳定

## 快速测试命令

```powershell
# 一键编译并运行（Visual Studio）
mkdir build -ErrorAction SilentlyContinue; cd build; cmake .. -G "Visual Studio 17 2022" -A x64; cmake --build . --config Release; cd ..; .\build\bin\Release\app_state_monitor.exe
```


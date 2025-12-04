# 配置化功能验证指南

## 一、应用分类配置化验证

### 1.1 验证配置文件加载

#### 步骤1：检查默认配置文件
```powershell
# 查看配置文件是否存在
Get-Content app_category_config.txt
```

#### 步骤2：运行程序，查看加载提示
```powershell
.\build\bin\Release\app_state_monitor.exe
```

**预期输出**：
- 如果配置文件存在且加载成功：
  ```
  已从配置文件加载应用分类: app_category_config.txt
  ```
- 如果配置文件不存在：
  ```
  提示: 未找到配置文件 "app_category_config.txt"，使用默认应用分类映射
  ```

### 1.2 验证自定义配置文件

#### 步骤1：创建测试配置文件
创建文件 `test_config.txt`：
```
# 测试配置文件
testapp.exe=GAME
mybrowser.exe=BROWSER
```

#### 步骤2：使用自定义配置文件运行
```powershell
.\build\bin\Release\app_state_monitor.exe --config test_config.txt
```

**预期输出**：
```
已从配置文件加载应用分类: test_config.txt
```

#### 步骤3：验证配置是否生效
- 如果系统中有 `testapp.exe` 进程，应该被分类为"游戏类"
- 如果系统中有 `mybrowser.exe` 进程，应该被分类为"浏览器/上网类"

### 1.3 验证配置覆盖默认映射

#### 步骤1：修改配置文件
编辑 `app_category_config.txt`，添加或修改一行：
```
chrome.exe=GAME  # 将Chrome改为游戏类（仅用于测试）
```

#### 步骤2：运行程序
```powershell
.\build\bin\Release\app_state_monitor.exe
```

#### 步骤3：打开Chrome浏览器
- 观察程序输出中的"应用类别"
- **预期结果**：应该显示"游戏类"而不是"浏览器/上网类"

#### 步骤4：恢复配置
测试完成后，将配置文件改回：
```
chrome.exe=BROWSER
```

### 1.4 验证配置文件格式错误处理

#### 测试1：格式错误的行
创建 `error_config.txt`：
```
# 格式错误示例
invalid_line
=GAME
chrome.exe
chrome.exe=INVALID_CATEGORY
chrome.exe=BROWSER  # 这行是正确的
```

运行程序：
```powershell
.\build\bin\Release\app_state_monitor.exe --config error_config.txt
```

**预期行为**：
- 程序应该跳过格式错误的行
- 只加载正确的行（`chrome.exe=BROWSER`）
- 如果所有行都错误，使用默认映射

#### 测试2：空配置文件
创建 `empty_config.txt`（空文件）：
```powershell
New-Item -Path empty_config.txt -ItemType File
.\build\bin\Release\app_state_monitor.exe --config empty_config.txt
```

**预期行为**：
- 程序应该使用默认映射
- 输出提示：未找到配置文件或使用默认映射

### 1.5 验证配置文件不存在的情况

#### 步骤1：删除或重命名配置文件
```powershell
Rename-Item app_category_config.txt app_category_config.txt.bak
```

#### 步骤2：运行程序
```powershell
.\build\bin\Release\app_state_monitor.exe
```

#### 步骤3：验证默认映射
- 程序应该正常启动
- 应该使用硬编码的默认映射
- 应用分类应该仍然正常工作

#### 步骤4：恢复配置文件
```powershell
Rename-Item app_category_config.txt.bak app_category_config.txt
```

---

## 二、验证新应用分类

### 2.1 添加新应用分类

#### 步骤1：查找进程名
打开任务管理器，找到要添加的应用的进程名（如 `notepad.exe`）

#### 步骤2：编辑配置文件
在 `app_category_config.txt` 中添加：
```
notepad.exe=DEVELOPMENT  # 将记事本分类为开发类（仅用于测试）
```

#### 步骤3：运行程序
```powershell
.\build\bin\Release\app_state_monitor.exe
```

#### 步骤4：打开记事本
- 观察程序输出
- **预期结果**：应用类别应该显示为"开发/编程类"

#### 步骤5：验证优先级
- 配置文件中的映射优先级高于关键词匹配
- 即使进程名或窗口标题中有其他关键词，也会使用配置文件中的分类

### 2.2 验证大小写不敏感

#### 测试配置
创建 `case_test.txt`：
```
CHROME.EXE=BROWSER
Firefox.exe=GAME
msedge.EXE=VIDEO
```

运行程序：
```powershell
.\build\bin\Release\app_state_monitor.exe --config case_test.txt
```

**预期行为**：
- 所有大小写变体都应该被正确识别
- 配置文件会自动转换为小写进行比较

---

## 三、验证规则配置（当前为硬编码）

### 3.1 查看当前规则

规则配置在 `main.cpp` 的 `InitializeRules()` 函数中。

### 3.2 验证规则优先级

#### 测试场景：同时满足多个规则
1. 在23:00-07:00之间（夜间）
2. 打开游戏应用
3. 观察灯光模式

**预期结果**：
- 应该显示"夜间弱光"（优先级10）
- 而不是"游戏/屏幕同步"（优先级8）
- 因为夜间规则优先级更高

### 3.3 修改规则进行测试

#### 示例：提高音频规则优先级
修改 `main.cpp` 中规则9的优先级：
```cpp
// 规则9: 有音频活动，使用音乐律动模式（最高优先级）
rule = Rule();
rule.priority = 100;  // 改为100，确保最高优先级
```

重新编译并运行：
```powershell
cd build
cmake --build . --config Release
cd ..
.\build\bin\Release\app_state_monitor.exe
```

**验证**：
- 播放音频时，应该优先显示"音乐律动"模式
- 即使其他规则也满足，音频规则应该优先

---

## 四、完整验证流程

### 4.1 配置文件加载验证

```powershell
# 1. 检查配置文件存在
Test-Path app_category_config.txt

# 2. 查看配置文件内容
Get-Content app_category_config.txt | Select-Object -First 10

# 3. 运行程序，查看加载提示
.\build\bin\Release\app_state_monitor.exe
```

### 4.2 配置生效验证

```powershell
# 1. 添加测试配置
Add-Content app_category_config.txt "notepad.exe=GAME"

# 2. 运行程序
.\build\bin\Release\app_state_monitor.exe

# 3. 打开记事本，观察应用类别
# 应该显示"游戏类"而不是默认分类

# 4. 恢复配置（删除测试行）
# 手动编辑配置文件删除测试行
```

### 4.3 自定义配置文件验证

```powershell
# 1. 创建测试配置
@"
testapp1.exe=GAME
testapp2.exe=VIDEO
"@ | Out-File -FilePath test_config.txt -Encoding UTF8

# 2. 使用自定义配置运行
.\build\bin\Release\app_state_monitor.exe --config test_config.txt

# 3. 验证加载提示
# 应该显示：已从配置文件加载应用分类: test_config.txt
```

---

## 五、常见问题验证

### 5.1 配置文件路径错误

```powershell
# 使用不存在的配置文件
.\build\bin\Release\app_state_monitor.exe --config nonexistent.txt
```

**预期行为**：
- 程序应该正常启动
- 使用默认映射
- 输出提示：未找到配置文件

### 5.2 配置文件格式错误

创建包含错误的配置文件，验证程序是否能正确处理。

### 5.3 配置文件编码问题

确保配置文件使用UTF-8编码，避免中文注释乱码。

---

## 六、快速验证清单

- [ ] 默认配置文件存在且可读
- [ ] 程序启动时显示配置文件加载提示
- [ ] 配置文件不存在时使用默认映射
- [ ] 自定义配置文件路径正常工作
- [ ] 配置文件中的映射覆盖默认映射
- [ ] 格式错误的行被正确跳过
- [ ] 大小写不敏感正常工作
- [ ] 添加新应用分类后生效
- [ ] 配置文件修改后需要重启程序才能生效

---

## 七、验证示例脚本

创建 `verify_config.ps1`：

```powershell
# 配置化功能验证脚本

Write-Host "=== 配置化功能验证 ===" -ForegroundColor Green

# 1. 检查配置文件
Write-Host "`n1. 检查配置文件..." -ForegroundColor Yellow
if (Test-Path "app_category_config.txt") {
    Write-Host "✓ 配置文件存在" -ForegroundColor Green
    $lineCount = (Get-Content app_category_config.txt | Where-Object { $_ -notmatch '^\s*#' -and $_ -notmatch '^\s*$' }).Count
    Write-Host "  配置项数量: $lineCount" -ForegroundColor Cyan
} else {
    Write-Host "✗ 配置文件不存在" -ForegroundColor Red
}

# 2. 测试自定义配置文件
Write-Host "`n2. 测试自定义配置文件..." -ForegroundColor Yellow
$testConfig = "test_verify_config.txt"
@"
# 测试配置
testapp.exe=GAME
"@ | Out-File -FilePath $testConfig -Encoding UTF8

Write-Host "  创建测试配置文件: $testConfig" -ForegroundColor Cyan

# 3. 运行程序（短暂运行）
Write-Host "`n3. 运行程序验证..." -ForegroundColor Yellow
Write-Host "  请手动运行程序验证配置加载" -ForegroundColor Cyan
Write-Host "  命令: .\build\bin\Release\app_state_monitor.exe --config $testConfig" -ForegroundColor Cyan

# 清理
Remove-Item $testConfig -ErrorAction SilentlyContinue

Write-Host "`n验证完成！" -ForegroundColor Green
```

运行验证脚本：
```powershell
.\verify_config.ps1
```



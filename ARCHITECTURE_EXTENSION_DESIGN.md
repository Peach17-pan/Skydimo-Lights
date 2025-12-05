# 架构扩展设计文档

## 一、设计原则

### 1.1 扩展性原则
- **向后兼容**：新条件类型的添加不应破坏现有功能
- **最小修改**：添加新条件类型时，只需修改必要的地方
- **统一接口**：所有条件类型使用统一的检查接口
- **数据驱动**：条件值使用统一的数据结构存储

### 1.2 当前架构分析

**优点**：
- 结构清晰，易于理解
- 类型安全（使用枚举）
- 性能好（switch-case）

**扩展点**：
- `ConditionType` 枚举：需要添加新类型
- `SystemState` 结构：需要添加新字段
- `Condition` 结构：需要添加新字段
- `CheckCondition()` 函数：需要添加新case

---

## 二、未来条件类型设计

### 2.1 硬件监控类条件

#### 2.1.1 CPU/GPU温度
```cpp
// 条件类型
enum class ConditionType {
    // ... 现有类型 ...
    CPU_TEMPERATURE,    // CPU温度阈值
    GPU_TEMPERATURE,    // GPU温度阈值
};

// SystemState扩展
struct SystemState {
    // ... 现有字段 ...
    double cpu_temperature;    // CPU温度（摄氏度）
    double gpu_temperature;    // GPU温度（摄氏度）
};

// Condition扩展
struct Condition {
    // ... 现有字段 ...
    std::optional<double> cpu_temp_threshold;      // CPU温度阈值
    bool cpu_temp_greater_than;                    // 是否大于阈值
    std::optional<double> gpu_temp_threshold;      // GPU温度阈值
    bool gpu_temp_greater_than;                    // 是否大于阈值
};
```

**实现方式**：
- 使用WMI或第三方库（如Open Hardware Monitor API）获取温度
- 创建 `TemperatureMonitor` 类，类似 `CpuMonitor`
- 在主循环中定期获取温度并更新 `SystemState`

#### 2.1.2 GPU使用率
```cpp
// 条件类型
enum class ConditionType {
    // ... 现有类型 ...
    GPU_USAGE_THRESHOLD,  // GPU使用率阈值
};

// SystemState扩展
struct SystemState {
    // ... 现有字段 ...
    double gpu_usage;  // GPU使用率 (0-100)
};

// Condition扩展
struct Condition {
    // ... 现有字段 ...
    std::optional<double> gpu_usage_threshold;  // GPU使用率阈值
    bool gpu_usage_greater_than;                // 是否大于阈值
};
```

**实现方式**：
- 使用NVIDIA/AMD API或WMI获取GPU使用率
- 创建 `GpuMonitor` 类
- 类似CPU使用率的实现方式

### 2.2 系统状态类条件

#### 2.2.1 锁屏/解锁状态
```cpp
// 条件类型
enum class ConditionType {
    // ... 现有类型 ...
    SCREEN_LOCK_STATE,  // 锁屏状态
};

// SystemState扩展
struct SystemState {
    // ... 现有字段 ...
    bool is_screen_locked;  // 是否锁屏（true=锁屏，false=解锁）
};

// Condition扩展
struct Condition {
    // ... 现有字段 ...
    std::optional<bool> screen_locked;  // 期望的锁屏状态
};
```

**实现方式**：
- 使用Windows API检测锁屏状态
  - 方法1：`GetForegroundWindow()` 检查是否有锁屏窗口
  - 方法2：使用 `WTSQuerySessionInformation` 检测会话状态
  - 方法3：监听 `WM_WTSSESSION_CHANGE` 消息
- 创建 `ScreenLockMonitor` 类
- 可以事件驱动（监听系统消息）或轮询

#### 2.2.2 系统主题（浅色/深色）
```cpp
// 条件类型
enum class ConditionType {
    // ... 现有类型 ...
    SYSTEM_THEME,  // 系统主题
};

// 主题类型枚举
enum class SystemTheme {
    LIGHT,  // 浅色主题
    DARK    // 深色主题
};

// SystemState扩展
struct SystemState {
    // ... 现有字段 ...
    SystemTheme system_theme;  // 当前系统主题
};

// Condition扩展
struct Condition {
    // ... 现有字段 ...
    std::optional<SystemTheme> expected_theme;  // 期望的主题
};
```

**实现方式**：
- Windows 10/11：读取注册表 `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize`
  - `AppsUseLightTheme` 键值
- 创建 `ThemeMonitor` 类
- 可以监听注册表变化或定期轮询

#### 2.2.3 昼夜判断（基于系统主题）
```cpp
// 可以复用系统主题条件，或单独实现
// 如果系统主题是深色，通常认为是"夜间模式"
// 也可以结合时间判断（如系统时间+系统主题）
```

### 2.3 外部指令类条件（特殊设计）

外部指令是**事件驱动**的，不同于其他轮询式条件，需要特殊设计。

#### 2.3.1 设计思路

**问题**：外部指令是异步事件，不能通过轮询 `SystemState` 获取。

**解决方案**：使用**事件/回调机制** + **状态缓存**

#### 2.3.2 架构设计

```cpp
// 条件类型
enum class ConditionType {
    // ... 现有类型 ...
    EXTERNAL_COMMAND,  // 外部指令
};

// 外部指令类型枚举
enum class ExternalCommandType {
    HARDWARE_BUTTON,    // 硬件按键
    MOBILE_CONTROL,     // 手机控制
    NETWORK_CONTROL,    // 网络控制
    STREAM_CHAT,        // 直播弹幕
    CUSTOM              // 自定义指令
};

// 外部指令结构
struct ExternalCommand {
    ExternalCommandType type;
    std::string command_id;      // 指令ID（如按键名称、弹幕关键词等）
    std::string parameter;        // 参数（可选）
    std::chrono::system_clock::time_point timestamp;  // 时间戳
};

// SystemState扩展（缓存最近的外部指令）
struct SystemState {
    // ... 现有字段 ...
    std::optional<ExternalCommand> last_external_command;  // 最近的外部指令
    std::chrono::system_clock::time_point last_command_time;  // 最后指令时间
};

// Condition扩展
struct Condition {
    // ... 现有字段 ...
    std::optional<ExternalCommandType> command_type;  // 指令类型
    std::optional<std::string> command_id;             // 指令ID（如按键名称）
    int command_timeout_seconds;                      // 指令有效期（秒），超时后失效
};
```

#### 2.3.3 外部指令管理器

```cpp
/**
 * 外部指令管理器
 * 负责接收和处理外部指令
 */
class ExternalCommandManager {
public:
    ExternalCommandManager();
    
    /**
     * 注册外部指令回调
     * @param callback 回调函数，当收到指令时调用
     */
    void SetCommandCallback(std::function<void(const ExternalCommand&)> callback);
    
    /**
     * 接收外部指令
     * @param command 外部指令
     */
    void ReceiveCommand(const ExternalCommand& command);
    
    /**
     * 获取当前有效的指令（在有效期内）
     * @return 有效的指令，如果无有效指令则返回nullopt
     */
    std::optional<ExternalCommand> GetActiveCommand() const;
    
    /**
     * 启动监听（后台线程）
     */
    void StartListening();
    
    /**
     * 停止监听
     */
    void StopListening();

private:
    std::optional<ExternalCommand> last_command_;
    std::chrono::system_clock::time_point last_command_time_;
    std::function<void(const ExternalCommand&)> command_callback_;
    std::thread listener_thread_;
    std::atomic<bool> running_;
    
    // 各种监听器的实现
    void ListenHardwareButton();    // 监听硬件按键
    void ListenMobileControl();     // 监听手机控制
    void ListenNetworkControl();    // 监听网络控制
    void ListenStreamChat();        // 监听直播弹幕
};
```

#### 2.3.4 实现方式

**硬件按键**：
- 使用Windows Hook API（`SetWindowsHookEx`）监听全局按键
- 或使用Raw Input API

**手机/网络控制**：
- 创建HTTP服务器（如使用libevent、cpp-httplib等）
- 接收RESTful API请求
- 或使用WebSocket实时通信

**直播弹幕控制**：
- 连接直播平台API（如B站、Twitch等）
- 解析弹幕消息
- 提取关键词作为指令

---

## 三、扩展实现步骤

### 3.1 添加新条件类型的标准流程

#### 步骤1：扩展枚举
在 `rule_engine.h` 的 `ConditionType` 枚举中添加新类型：
```cpp
enum class ConditionType {
    // ... 现有类型 ...
    NEW_CONDITION_TYPE,  // 新条件类型
};
```

#### 步骤2：扩展SystemState
在 `SystemState` 结构中添加新字段：
```cpp
struct SystemState {
    // ... 现有字段 ...
    NewDataType new_field;  // 新字段
};
```

#### 步骤3：扩展Condition
在 `Condition` 结构中添加新字段：
```cpp
struct Condition {
    // ... 现有字段 ...
    std::optional<NewDataType> new_condition_field;  // 新条件字段
};
```

#### 步骤4：实现检查逻辑
在 `CheckCondition()` 函数中添加新case：
```cpp
case ConditionType::NEW_CONDITION_TYPE:
    if (condition.new_condition_field.has_value()) {
        // 实现检查逻辑
        return CheckNewCondition(condition, state);
    }
    return false;
```

#### 步骤5：创建监控类（如需要）
创建新的监控类（如 `TemperatureMonitor`、`GpuMonitor` 等），在主循环中调用。

#### 步骤6：更新规则初始化
在 `InitializeRules()` 中添加使用新条件的示例规则。

### 3.2 外部指令的特殊实现

#### 步骤1：创建ExternalCommandManager
创建 `external_command_manager.h` 和 `external_command_manager.cpp`

#### 步骤2：集成到主程序
```cpp
// main.cpp
ExternalCommandManager cmd_manager;
cmd_manager.SetCommandCallback([&](const ExternalCommand& cmd) {
    // 更新SystemState中的last_external_command
    // 触发规则重新评估
});
cmd_manager.StartListening();
```

#### 步骤3：在主循环中检查
```cpp
// 检查是否有有效的外部指令
auto active_cmd = cmd_manager.GetActiveCommand();
if (active_cmd.has_value()) {
    system_state.last_external_command = active_cmd;
    system_state.last_command_time = std::chrono::system_clock::now();
}
```

---

## 四、架构优化建议

### 4.1 条件检查的扩展性优化

**当前方式**（switch-case）：
- 优点：性能好，类型安全
- 缺点：添加新类型需要修改多个地方

**可选优化方案**（保持向后兼容）：

#### 方案1：策略模式（可选，未来重构）
```cpp
// 条件检查器接口
class ConditionChecker {
public:
    virtual ~ConditionChecker() = default;
    virtual bool Check(const Condition& condition, const SystemState& state) = 0;
};

// 具体检查器
class AppCategoryChecker : public ConditionChecker { ... };
class TemperatureChecker : public ConditionChecker { ... };

// 注册机制
class RuleEngine {
    std::unordered_map<ConditionType, std::unique_ptr<ConditionChecker>> checkers_;
public:
    void RegisterChecker(ConditionType type, std::unique_ptr<ConditionChecker> checker);
};
```

**优点**：添加新类型只需创建新检查器类，无需修改RuleEngine
**缺点**：需要重构现有代码

#### 方案2：保持当前方式（推荐）
- 保持switch-case方式
- 添加新类型时按标准流程修改
- 简单直接，易于维护

### 4.2 SystemState的扩展性

**当前方式**：结构体，所有字段都在一个结构中

**扩展建议**：
- 保持当前方式（简单直接）
- 或使用组合模式（将相关字段分组）

```cpp
// 可选：分组结构
struct HardwareState {
    double cpu_usage;
    double cpu_temperature;
    double gpu_usage;
    double gpu_temperature;
};

struct SystemState {
    AppCategory current_app_category;
    HardwareState hardware;
    // ... 其他字段
};
```

### 4.3 Condition的扩展性

**当前方式**：使用 `std::optional` 存储不同类型的值

**扩展建议**：
- 保持当前方式（类型安全，易于理解）
- 或使用 `std::variant`（更灵活，但可能过度设计）

---

## 五、具体扩展设计

### 5.1 CPU/GPU温度扩展

**文件结构**：
```
temperature_monitor.h
temperature_monitor.cpp
```

**接口设计**：
```cpp
class TemperatureMonitor {
public:
    bool Initialize();
    double GetCpuTemperature();  // 返回摄氏度，失败返回-1
    double GetGpuTemperature();  // 返回摄氏度，失败返回-1
};
```

**实现方式**：
- Windows：使用WMI查询 `Win32_TemperatureProbe`
- 或使用第三方库（如Open Hardware Monitor、LibreHardwareMonitor）

### 5.2 GPU使用率扩展

**文件结构**：
```
gpu_monitor.h
gpu_monitor.cpp
```

**接口设计**：
```cpp
class GpuMonitor {
public:
    bool Initialize();
    double GetGpuUsage();  // 返回0-100，失败返回-1
};
```

**实现方式**：
- NVIDIA：使用NVAPI
- AMD：使用ADL API
- 通用：使用WMI或Performance Counters

### 5.3 锁屏状态扩展

**文件结构**：
```
screen_lock_monitor.h
screen_lock_monitor.cpp
```

**接口设计**：
```cpp
class ScreenLockMonitor {
public:
    bool Initialize();
    bool IsScreenLocked();  // 返回是否锁屏
    void SetLockStateCallback(std::function<void(bool)> callback);  // 事件回调
};
```

**实现方式**：
- 监听 `WM_WTSSESSION_CHANGE` 消息
- 或定期检查 `GetForegroundWindow()`

### 5.4 系统主题扩展

**文件结构**：
```
theme_monitor.h
theme_monitor.cpp
```

**接口设计**：
```cpp
enum class SystemTheme {
    LIGHT,
    DARK
};

class ThemeMonitor {
public:
    bool Initialize();
    SystemTheme GetSystemTheme();  // 获取当前主题
    void SetThemeChangeCallback(std::function<void(SystemTheme)> callback);  // 事件回调
};
```

**实现方式**：
- 读取注册表：`HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize\AppsUseLightTheme`
- 监听注册表变化

### 5.5 外部指令扩展

**文件结构**：
```
external_command_manager.h
external_command_manager.cpp
```

**接口设计**：
```cpp
class ExternalCommandManager {
public:
    void SetCommandCallback(std::function<void(const ExternalCommand&)> callback);
    void ReceiveCommand(const ExternalCommand& command);
    std::optional<ExternalCommand> GetActiveCommand(int timeout_seconds = 5) const;
    void StartListening();
    void StopListening();
    
    // 各种监听器的启动/停止
    void StartHardwareButtonListener();
    void StartNetworkServer(int port);
    void StartStreamChatListener(const std::string& platform, const std::string& room_id);
};
```

**实现方式**：
- 硬件按键：Windows Hook API
- 网络控制：HTTP服务器（如cpp-httplib）
- 直播弹幕：连接平台API（如B站、Twitch等）

---

## 六、扩展示例代码

### 6.1 添加CPU温度条件（伪代码）

```cpp
// 1. 扩展枚举
enum class ConditionType {
    // ... 现有类型 ...
    CPU_TEMPERATURE,
};

// 2. 扩展SystemState
struct SystemState {
    // ... 现有字段 ...
    double cpu_temperature;
};

// 3. 扩展Condition
struct Condition {
    // ... 现有字段 ...
    std::optional<double> cpu_temp_threshold;
    bool cpu_temp_greater_than;
};

// 4. 实现检查逻辑
case ConditionType::CPU_TEMPERATURE:
    if (condition.cpu_temp_threshold.has_value()) {
        if (condition.cpu_temp_greater_than) {
            return state.cpu_temperature > condition.cpu_temp_threshold.value();
        } else {
            return state.cpu_temperature <= condition.cpu_temp_threshold.value();
        }
    }
    return false;

// 5. 在主循环中获取温度
TemperatureMonitor temp_monitor;
temp_monitor.Initialize();
system_state.cpu_temperature = temp_monitor.GetCpuTemperature();
```

### 6.2 添加外部指令条件（伪代码）

```cpp
// 1. 扩展枚举和结构（见前面设计）

// 2. 在主程序中集成
ExternalCommandManager cmd_manager;
cmd_manager.SetCommandCallback([&](const ExternalCommand& cmd) {
    // 更新状态，触发规则重新评估
    system_state.last_external_command = cmd;
    system_state.last_command_time = std::chrono::system_clock::now();
});
cmd_manager.StartListening();

// 3. 在规则检查中
case ConditionType::EXTERNAL_COMMAND:
    if (condition.command_type.has_value() && 
        condition.command_id.has_value() &&
        state.last_external_command.has_value()) {
        const auto& cmd = state.last_external_command.value();
        
        // 检查类型和ID是否匹配
        if (cmd.type == condition.command_type.value() &&
            cmd.command_id == condition.command_id.value()) {
            
            // 检查是否在有效期内
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now() - cmd.timestamp).count();
            return elapsed < condition.command_timeout_seconds;
        }
    }
    return false;
```

---

## 七、架构兼容性保证

### 7.1 向后兼容
- 现有条件类型保持不变
- 现有规则继续工作
- 新字段使用 `std::optional`，默认不设置时不影响现有逻辑

### 7.2 最小修改原则
- 添加新条件类型只需修改：
  1. `ConditionType` 枚举（+1行）
  2. `SystemState` 结构（+1-2行）
  3. `Condition` 结构（+1-2行）
  4. `CheckCondition()` 函数（+5-10行）
  5. 创建监控类（新文件，不影响现有代码）

### 7.3 模块化设计
- 每个监控功能独立成类
- 通过 `SystemState` 统一传递数据
- 规则引擎不依赖具体的监控实现

---

## 八、总结

### 8.1 扩展性设计要点

1. **统一的数据结构**：`SystemState` 作为所有状态数据的容器
2. **统一的条件检查接口**：`CheckCondition()` 函数
3. **模块化监控**：每个监控功能独立实现
4. **事件驱动支持**：外部指令使用回调机制
5. **向后兼容**：新功能不影响现有功能

### 8.2 扩展难度评估

| 功能 | 难度 | 需要修改的文件 |
|------|------|----------------|
| CPU/GPU温度 | 中等 | 新增2文件，修改3处 |
| GPU使用率 | 中等 | 新增2文件，修改3处 |
| 锁屏状态 | 简单 | 新增2文件，修改3处 |
| 系统主题 | 简单 | 新增2文件，修改3处 |
| 外部指令 | 复杂 | 新增2文件，修改5处，需要事件机制 |

### 8.3 推荐实现顺序

1. **锁屏状态**（最简单，验证扩展流程）
2. **系统主题**（简单，验证扩展流程）
3. **CPU/GPU温度**（需要第三方库或WMI）
4. **GPU使用率**（需要硬件厂商API）
5. **外部指令**（最复杂，需要网络/事件处理）

---

## 九、注意事项

1. **性能考虑**：温度、GPU使用率等监控可能较耗时，考虑缓存机制
2. **错误处理**：硬件监控可能失败，需要优雅降级
3. **跨平台**：当前设计针对Windows，未来扩展时考虑跨平台
4. **线程安全**：外部指令管理器需要线程安全设计
5. **资源管理**：监听器需要正确启动和停止，避免资源泄漏


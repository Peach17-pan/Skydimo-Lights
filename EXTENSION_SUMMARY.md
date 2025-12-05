# 架构扩展设计总结

## 一、扩展概述

本次设计为规则引擎系统增加了**5类未来可扩展的条件类型**，在不大改现有架构的前提下，为系统预留了清晰的扩展路径。

---

## 二、新增条件类型设计

### 2.1 硬件监控类条件

#### 1. CPU/GPU温度监控
- **条件类型**：`CPU_TEMPERATURE`、`GPU_TEMPERATURE`
- **数据类型**：`double`（摄氏度）
- **条件逻辑**：支持大于/小于等于阈值判断
- **实现方式**：
  - 使用WMI查询 `Win32_TemperatureProbe`
  - 或使用第三方库（Open Hardware Monitor、LibreHardwareMonitor）
- **监控类**：`TemperatureMonitor`
- **扩展难度**：⭐⭐ 中等

#### 2. GPU使用率监控
- **条件类型**：`GPU_USAGE_THRESHOLD`
- **数据类型**：`double`（0-100%）
- **条件逻辑**：支持大于/小于等于阈值判断
- **实现方式**：
  - NVIDIA：使用NVAPI
  - AMD：使用ADL API
  - 通用：使用WMI或Performance Counters
- **监控类**：`GpuMonitor`
- **扩展难度**：⭐⭐ 中等

### 2.2 系统状态类条件

#### 3. 锁屏/解锁状态
- **条件类型**：`SCREEN_LOCK_STATE`
- **数据类型**：`bool`（true=锁屏，false=解锁）
- **条件逻辑**：匹配期望的锁屏状态
- **实现方式**：
  - 监听 `WM_WTSSESSION_CHANGE` 消息（事件驱动）
  - 或定期检查 `GetForegroundWindow()`（轮询）
- **监控类**：`ScreenLockMonitor`
- **扩展难度**：⭐ 简单

#### 4. 系统主题（浅色/深色）
- **条件类型**：`SYSTEM_THEME`
- **数据类型**：`SystemTheme` 枚举（LIGHT/DARK）
- **条件逻辑**：匹配期望的主题
- **实现方式**：
  - 读取注册表：`HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize\AppsUseLightTheme`
  - 监听注册表变化（事件驱动）或定期轮询
- **监控类**：`ThemeMonitor`
- **扩展难度**：⭐ 简单

#### 5. 昼夜判断
- **实现方式**：
  - 方案1：复用系统主题条件（深色主题=夜间）
  - 方案2：结合系统时间+系统主题判断
  - 方案3：单独实现，基于系统时间

### 2.3 外部指令类条件（特殊设计）

#### 6. 外部指令控制
- **条件类型**：`EXTERNAL_COMMAND`
- **指令类型**：
  - `HARDWARE_BUTTON` - 硬件按键
  - `MOBILE_CONTROL` - 手机控制
  - `NETWORK_CONTROL` - 网络控制
  - `STREAM_CHAT` - 直播弹幕
  - `CUSTOM` - 自定义指令
- **数据结构**：
  ```cpp
  struct ExternalCommand {
      ExternalCommandType type;
      std::string command_id;      // 指令ID
      std::string parameter;        // 参数
      std::chrono::system_clock::time_point timestamp;
  };
  ```
- **特殊设计**：
  - **事件驱动**：不同于其他轮询式条件
  - **回调机制**：使用回调函数接收指令
  - **状态缓存**：在 `SystemState` 中缓存最近的有效指令
  - **有效期机制**：指令有超时时间，超时后失效
- **实现方式**：
  - **硬件按键**：Windows Hook API（`SetWindowsHookEx`）或Raw Input API
  - **手机/网络控制**：HTTP服务器（如cpp-httplib）或WebSocket
  - **直播弹幕**：连接平台API（B站、Twitch等），解析弹幕消息
- **管理类**：`ExternalCommandManager`
- **扩展难度**：⭐⭐⭐ 复杂

---

## 三、架构扩展设计

### 3.1 标准扩展流程（5步法）

添加新条件类型的标准流程：

1. **扩展枚举**（+1行）
   ```cpp
   enum class ConditionType {
       // ... 现有类型 ...
       NEW_CONDITION_TYPE,
   };
   ```

2. **扩展SystemState**（+1-2行）
   ```cpp
   struct SystemState {
       // ... 现有字段 ...
       NewDataType new_field;
   };
   ```

3. **扩展Condition**（+1-2行）
   ```cpp
   struct Condition {
       // ... 现有字段 ...
       std::optional<NewDataType> new_condition_field;
   };
   ```

4. **实现检查逻辑**（+5-10行）
   ```cpp
   case ConditionType::NEW_CONDITION_TYPE:
       // 实现检查逻辑
       return CheckNewCondition(condition, state);
   ```

5. **创建监控类**（新文件，不影响现有代码）
   ```cpp
   class NewMonitor {
       // 监控实现
   };
   ```

### 3.2 外部指令的特殊架构

**设计要点**：
- 使用**事件/回调机制**而非轮询
- 在 `SystemState` 中缓存最近的有效指令
- 条件检查时验证指令是否在有效期内
- 支持多种指令源（按键、网络、弹幕等）

**架构组件**：
```
ExternalCommandManager
├── 指令接收（回调机制）
├── 指令缓存（SystemState）
├── 有效期管理
└── 多种监听器
    ├── 硬件按键监听
    ├── 网络服务器
    └── 直播弹幕监听
```

---

## 四、扩展性保证

### 4.1 向后兼容
- ✅ 现有条件类型保持不变
- ✅ 现有规则继续工作
- ✅ 新字段使用 `std::optional`，默认不影响现有逻辑

### 4.2 最小修改原则
- ✅ 添加新条件类型只需修改3-5处
- ✅ 每个监控功能独立成类（新文件）
- ✅ 通过 `SystemState` 统一传递数据

### 4.3 模块化设计
- ✅ 规则引擎不依赖具体监控实现
- ✅ 监控类可以独立开发、测试
- ✅ 易于维护和扩展

---

## 五、实现优先级建议

### 5.1 推荐实现顺序

1. **锁屏状态**（最简单，验证扩展流程）
   - 难度：⭐
   - 工作量：2-3小时
   - 价值：验证扩展机制

2. **系统主题**（简单，验证扩展流程）
   - 难度：⭐
   - 工作量：2-3小时
   - 价值：验证扩展机制

3. **CPU/GPU温度**（需要第三方库）
   - 难度：⭐⭐
   - 工作量：4-6小时
   - 价值：硬件监控

4. **GPU使用率**（需要硬件厂商API）
   - 难度：⭐⭐
   - 工作量：4-6小时
   - 价值：硬件监控

5. **外部指令**（最复杂，需要网络/事件处理）
   - 难度：⭐⭐⭐
   - 工作量：8-12小时
   - 价值：用户交互和控制

### 5.2 难度评估表

| 功能 | 难度 | 需要修改的文件 | 新增文件 | 预计工作量 |
|------|------|----------------|----------|-----------|
| 锁屏状态 | ⭐ | 3处 | 2个 | 2-3小时 |
| 系统主题 | ⭐ | 3处 | 2个 | 2-3小时 |
| CPU/GPU温度 | ⭐⭐ | 3处 | 2个 | 4-6小时 |
| GPU使用率 | ⭐⭐ | 3处 | 2个 | 4-6小时 |
| 外部指令 | ⭐⭐⭐ | 5处 | 2个 | 8-12小时 |

---

## 六、设计亮点

### 6.1 统一的数据流
```
监控类 → SystemState → RuleEngine → LightMode
```

### 6.2 灵活的条件组合
- 支持AND逻辑（多个条件同时满足）
- 未来可扩展OR逻辑、NOT逻辑

### 6.3 事件驱动支持
- 外部指令使用回调机制
- 锁屏、主题等支持事件监听
- 与轮询机制并存

### 6.4 类型安全
- 使用枚举确保类型安全
- 使用 `std::optional` 处理可选值
- 编译时检查，减少运行时错误

---

## 七、扩展示例

### 7.1 CPU温度条件示例

```cpp
// 1. 扩展枚举
enum class ConditionType {
    CPU_TEMPERATURE,
};

// 2. 扩展SystemState
struct SystemState {
    double cpu_temperature;
};

// 3. 扩展Condition
struct Condition {
    std::optional<double> cpu_temp_threshold;
    bool cpu_temp_greater_than;
};

// 4. 实现检查
case ConditionType::CPU_TEMPERATURE:
    if (condition.cpu_temp_threshold.has_value()) {
        return condition.cpu_temp_greater_than ?
            state.cpu_temperature > condition.cpu_temp_threshold.value() :
            state.cpu_temperature <= condition.cpu_temp_threshold.value();
    }
    return false;
```

### 7.2 外部指令条件示例

```cpp
// 1. 扩展枚举和结构
enum class ConditionType {
    EXTERNAL_COMMAND,
};

struct ExternalCommand {
    ExternalCommandType type;
    std::string command_id;
    std::chrono::system_clock::time_point timestamp;
};

// 2. 扩展SystemState
struct SystemState {
    std::optional<ExternalCommand> last_external_command;
};

// 3. 实现检查（带有效期）
case ConditionType::EXTERNAL_COMMAND:
    if (state.last_external_command.has_value()) {
        const auto& cmd = state.last_external_command.value();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now() - cmd.timestamp).count();
        return elapsed < condition.command_timeout_seconds &&
               cmd.type == condition.command_type.value() &&
               cmd.command_id == condition.command_id.value();
    }
    return false;
```

---

## 八、技术要点

### 8.1 Windows API使用

| 功能 | 使用的API/技术 |
|------|----------------|
| CPU/GPU温度 | WMI (`Win32_TemperatureProbe`) |
| GPU使用率 | NVAPI / ADL API / WMI |
| 锁屏状态 | `WM_WTSSESSION_CHANGE` 消息 |
| 系统主题 | 注册表读取 |
| 硬件按键 | `SetWindowsHookEx` / Raw Input |
| 网络控制 | HTTP服务器（cpp-httplib等） |
| 直播弹幕 | 平台API（B站、Twitch等） |

### 8.2 设计模式

- **策略模式**（可选）：条件检查器可抽象为策略
- **观察者模式**：外部指令使用回调机制
- **单例模式**（可选）：监控类可设计为单例

### 8.3 性能考虑

- **缓存机制**：温度、GPU等监控使用缓存，避免频繁查询
- **事件驱动**：锁屏、主题等优先使用事件，减少轮询
- **异步处理**：外部指令使用异步处理，不阻塞主循环

---

## 九、总结

### 9.1 扩展设计成果

✅ **5类新条件类型**的完整设计  
✅ **标准扩展流程**（5步法）  
✅ **外部指令特殊架构**（事件驱动）  
✅ **向后兼容保证**  
✅ **最小修改原则**  
✅ **模块化设计**  

### 9.2 架构优势

1. **易于扩展**：添加新类型只需3-5处修改
2. **类型安全**：使用枚举和 `std::optional`
3. **性能良好**：switch-case编译时优化
4. **易于维护**：结构清晰，职责明确
5. **向后兼容**：新功能不影响现有功能

### 9.3 实现准备

- ✅ 设计文档完整
- ✅ 扩展示例清晰
- ✅ 技术方案明确
- ✅ 实现难度评估
- ✅ 优先级建议

所有设计都遵循"不大改"的原则，保持现有架构稳定，同时为未来扩展做好充分准备。


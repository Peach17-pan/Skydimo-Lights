# 规则引擎系统说明文档

## 概述

本系统实现了一个基于规则的灯光模式决策引擎，能够根据当前系统状态（应用类别、时间段、CPU使用率、用户空闲时间）自动决定应该使用的灯光模式。

## 对外接口

### 获取当前灯光模式

系统提供 `RuleEngine::DecideLightMode()` 函数作为对外接口，供上层定期调用：

```cpp
// 构建系统状态
SystemState system_state;
system_state.current_app_category = category;
system_state.current_hour = current_hour;
system_state.current_minute = current_minute;
system_state.cpu_usage = cpu_usage;
system_state.idle_minutes = idle_minutes;

// 获取当前判定的灯光模式
LightMode light_mode = rule_engine.DecideLightMode(system_state);
```

### 模式变化检测

Demo程序会自动检测灯光模式的变化，当模式发生变化时会输出：
```
>>> Mode changed to: XXX (from YYY)
```

## 支持的条件类型

### 1. 应用类别 (APP_CATEGORY)

根据当前前台应用的类别进行判断。

**支持的应用类别**：
- `GAME` - 游戏类
- `VIDEO` - 电影/视频类
- `MUSIC` - 音乐类
- `DOCUMENT` - 文档/办公类
- `BROWSER` - 浏览器/上网类
- `DEVELOPMENT` - 开发/编程类
- `CREATIVE` - 创作类
- `UNKNOWN` - 未知

**规则示例**：
```cpp
Condition condition;
condition.type = ConditionType::APP_CATEGORY;
condition.app_category = AppCategory::GAME;
```

### 2. 时间段 (TIME_RANGE)

根据当前时间是否在指定时间段内进行判断。支持跨天时间段（如 23:00-07:00）。

**规则示例**：
```cpp
Condition condition;
condition.type = ConditionType::TIME_RANGE;
condition.time_range = TimeRange(23, 0, 7, 0);  // 23:00-07:00
```

### 3. CPU使用率阈值 (CPU_THRESHOLD)

根据当前CPU使用率是否超过阈值进行判断。

**规则示例**：
```cpp
Condition condition;
condition.type = ConditionType::CPU_THRESHOLD;
condition.cpu_threshold = 80.0;        // 80%
condition.cpu_greater_than = true;      // > 80% (false表示 <= 80%)
```

### 4. Idle时间阈值 (IDLE_THRESHOLD)

根据用户空闲时间（从上次键盘/鼠标操作起）是否超过阈值进行判断。

**规则示例**：
```cpp
Condition condition;
condition.type = ConditionType::IDLE_THRESHOLD;
condition.idle_threshold = 10.0;        // 10分钟
condition.idle_greater_than = true;     // >= 10分钟 (false表示 < 10分钟)
```

## 条件组合

当前版本支持**AND逻辑**：规则中的所有条件必须同时满足才会触发。

**示例**：
```cpp
Rule rule;
rule.priority = 5;
rule.target_mode = LightMode::WORK_CODING;

// 条件1：应用类别为开发类
Condition condition1;
condition1.type = ConditionType::APP_CATEGORY;
condition1.app_category = AppCategory::DEVELOPMENT;
rule.conditions.push_back(condition1);

// 条件2：CPU使用率 < 50%
Condition condition2;
condition2.type = ConditionType::CPU_THRESHOLD;
condition2.cpu_threshold = 50.0;
condition2.cpu_greater_than = false;  // <= 50%
rule.conditions.push_back(condition2);

// 只有当两个条件都满足时，才会触发此规则
```

## 灯光模式

系统支持以下灯光模式：

| 模式 | 枚举值 | 说明 |
|------|--------|------|
| 游戏/屏幕同步 | `GAME_SCREENSYNC` | 游戏时使用，与屏幕内容同步 |
| 影视模式 | `VIDEO_CINEMATIC` | 观看视频时使用 |
| 音乐律动 | `MUSIC` | 听音乐时使用，随音乐律动 |
| 办公/写代码 | `WORK_CODING` | 办公或编程时使用，静态模式 |
| 夜间弱光 | `NIGHT_DIM` | 夜间使用，降低亮度 |
| 关闭灯光 | `OFF` | 关闭所有灯光 |
| 默认模式 | `DEFAULT` | 默认模式，无规则匹配时使用 |

## 优先级机制

当多个规则同时满足时，系统按规则的**优先级**决定最终模式：
- **优先级数值越大，优先级越高**
- 系统按优先级从高到低检查规则
- 第一个所有条件都满足的规则会被采用

**当前预配置规则优先级**：

| 优先级 | 条件 | 灯光模式 |
|--------|------|----------|
| 10 | 23:00-07:00 | 夜间弱光 |
| 9 | 空闲 ≥ 10分钟 | 关闭灯光 |
| 8 | 游戏类应用 | 游戏/屏幕同步 |
| 7 | 视频类应用 | 影视模式 |
| 6 | 音乐类应用 | 音乐律动 |
| 5 | 开发/编程类应用 | 办公/写代码 |
| 4 | 文档/办公类应用 | 办公/写代码 |
| 3 | CPU使用率 > 80% | 游戏/屏幕同步 |
| 0 | 默认（无匹配） | 默认模式 |

## 状态采集频率

- **默认采集间隔**：3秒（3000毫秒）
- **可通过命令行参数自定义**：`app_state_monitor.exe <间隔毫秒数>`
- **示例**：`app_state_monitor.exe 1000` （每1秒采集一次）

每次采集会获取：
- 当前前台应用信息（进程名、窗口标题、应用类别）
- 当前时间（小时、分钟）
- CPU使用率（0-100%）
- 用户空闲时间（分钟）

## 规则配置方式

### 当前实现：代码中硬编码

规则目前通过 `main.cpp` 中的 `InitializeRules()` 函数进行配置，规则直接写在代码中。

**配置位置**：`main.cpp` 的 `InitializeRules()` 函数

**配置示例**：
```cpp
void InitializeRules(RuleEngine& rule_engine) {
    Rule rule;
    
    // 规则：夜间弱光模式
    rule = Rule();
    rule.priority = 10;
    rule.target_mode = LightMode::NIGHT_DIM;
    Condition time_condition;
    time_condition.type = ConditionType::TIME_RANGE;
    time_condition.time_range = TimeRange(23, 0, 7, 0);
    rule.conditions.push_back(time_condition);
    rule_engine.AddRule(rule);
    
    // 添加更多规则...
}
```

### 未来扩展方向

未来可以考虑：
- 从配置文件（JSON/YAML）加载规则
- 通过API动态添加/删除规则
- 提供图形界面进行规则配置

## Demo程序使用说明

### 运行Demo

```powershell
.\build\bin\Release\app_state_monitor.exe
```

### 输出内容

Demo程序每隔3秒（或自定义间隔）输出一次当前状态：

```
[2024-01-15 14:30:25.123]
  应用类别: 开发/编程类
  当前时间: 14:30 (周一)
  CPU使用率: 45.2%
  Idle时间: 0.5 分钟
  当前判定的灯光模式: 办公/写代码
------------------------------------------------------------
```

### 模式变化提示

当灯光模式发生变化时，会清晰输出：

```
>>> Mode changed to: 游戏/屏幕同步 (from 办公/写代码)
```

### 调试模式

使用 `--debug` 参数可以显示更详细的调试信息：

```powershell
.\build\bin\Release\app_state_monitor.exe --debug
```

## 文件结构

```
.
├── rule_engine.h          # 规则引擎头文件
├── rule_engine.cpp        # 规则引擎实现
├── main.cpp               # Demo程序主文件（包含规则初始化）
├── app_classifier.h       # 应用分类器头文件
├── app_classifier.cpp     # 应用分类器实现
├── window_monitor.h       # 窗口监控器头文件
├── window_monitor.cpp     # 窗口监控器实现
└── RULE_ENGINE_README.md  # 本说明文档
```

## 测试建议

1. **测试应用类别规则**：打开不同类型的应用（游戏、视频、代码编辑器等），观察灯光模式是否相应变化
2. **测试空闲时间规则**：停止操作键盘和鼠标，等待10分钟，观察是否切换到"关闭灯光"模式
3. **测试CPU使用率规则**：运行CPU密集型任务，观察CPU使用率超过80%时是否切换到相应模式
4. **测试时间段规则**：在23:00-07:00之间运行程序，观察是否切换到"夜间弱光"模式
5. **测试优先级**：同时满足多个规则条件时，观察是否按优先级选择正确的模式

## 注意事项

1. **CPU使用率计算**：第一次调用可能返回0，需要等待第二次调用才能获得准确值
2. **空闲时间**：从程序启动时开始计算，如果程序启动后立即操作，空闲时间会重置
3. **时间段跨天**：支持跨天时间段（如23:00-07:00），系统会自动处理
4. **规则匹配**：如果没有规则匹配，系统会返回默认模式（DEFAULT）

## 扩展开发

如需添加新的条件类型或灯光模式：

1. 在 `rule_engine.h` 中添加新的枚举值
2. 在 `rule_engine.cpp` 的 `CheckCondition()` 函数中添加相应的判断逻辑
3. 在 `InitializeRules()` 中添加使用新条件的示例规则


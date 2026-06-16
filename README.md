# 汽车仪表盘系统 - 完整测试流程

## 1. 项目概述

本项目包含两个独立的 Qt 桌面应用，共同构成一套完整的汽车仪表盘仿真系统：

| 应用 | 目录 | 说明 |
|:----|:-----|:-----|
| **final_simulator** | `final_simulator/` | 信号模拟器，TCP 服务端，以 100ms 为周期广播车辆信号 JSON 数据 |
| **dashboard** | `work/dashboard/` | 仪表盘前端，连接模拟器或使用 Mock 模式独立显示车辆状态 |

- 开发环境：Qt 5.3.2 + MinGW 4.8.2 (C++11)
- 通信协议：TCP/IP，JSON 行协议，端口 5000
- 构建系统：qmake + mingw32-make

## 2. 前置条件

### 2.1 安装依赖

| 组件 | 路径 |
|:----|:-----|
| Qt 5.3.2 | `D:\QT\Qt5.3.2\5.3\mingw482_32\` |
| MinGW 4.8.2 编译器 | `D:\QT\Qt5.3.2\Tools\mingw482_32\bin\` |
| qmake | `D:\QT\Qt5.3.2\5.3\mingw482_32\bin\qmake.exe` |

### 2.2 环境变量

构建和运行前需要将 Qt 工具链加入 PATH。系统默认 PATH 中可能包含新版 Qt（如 Anaconda 带的 5.15），请确保以下路径优先：

```cmd
set PATH=D:\QT\Qt5.3.2\5.3\mingw482_32\bin;D:\QT\Qt5.3.2\Tools\mingw482_32\bin;%PATH%
```

### 2.3 插件配置 (qt.conf)

Qt 5.3.2 需要找到 `platforms/qwindows.dll` 才能创建窗口。构建完成后，在输出目录（`debug/` 或 `release/`）下应包含 `qt.conf`：

```ini
[Paths]
Prefix = D:/QT/Qt5.3.2/5.3/mingw482_32
Plugins = plugins
```

如果缺失此文件，exe 启动后会立即退出且无任何提示。

## 3. 构建方法

### 3.1 构建模拟器

```cmd
cd final_simulator
set PATH=D:\QT\Qt5.3.2\5.3\mingw482_32\bin;D:\QT\Qt5.3.2\Tools\mingw482_32\bin;%PATH%
set QMAKESPEC=win32-g++
qmake final_simulator.pro -spec win32-g++
mingw32-make -j4
```

产物：`final_simulator/release/final_simulator.exe`

### 3.2 构建仪表盘

```cmd
cd work\dashboard
set PATH=D:\QT\Qt5.3.2\5.3\mingw482_32\bin;D:\QT\Qt5.3.2\Tools\mingw482_32\bin;%PATH%
set QMAKESPEC=win32-g++
qmake dashboard.pro -spec win32-g++
mingw32-make -j4
```

产物：`work/dashboard/release/dashboard.exe`（或 `debug/dashboard.exe`）

> 仪表盘需要 `QT += widgets multimedia`，确保编译时多媒体模块可用。

## 4. 完整测试流程

### 4.1 启动模拟器

```cmd
cd final_simulator
set PATH=D:\QT\Qt5.3.2\5.3\mingw482_32\bin;D:\QT\Qt5.3.2\Tools\mingw482_32\bin;%PATH%
release\final_simulator.exe
```

预期控制台输出：

```
[sim] final_simulator listening on 0.0.0.0:5000 (waiting for dashboard clients)
[sim] final_simulator running. Press Ctrl+C to stop.
[sim] Dashboard clients should connect to 127.0.0.1:5000
```

模拟器默认监听端口 5000，支持多客户端连接。每 100ms 向所有已连接的客户端广播一帧 JSON 数据。

### 4.2 启动仪表盘

保持模拟器运行，新开一个命令行窗口：

```cmd
cd work\dashboard
set PATH=D:\QT\Qt5.3.2\5.3\mingw482_32\bin;D:\QT\Qt5.3.2\Tools\mingw482_32\bin;%PATH%
release\dashboard.exe
```

**预期结果**：

- 仪表盘窗口正常显示，包含速度表、转速表、油量表、水温表、转向指示灯和报警灯区域
- 窗口标题显示"汽车仪表盘 - Socket 模式"
- **状态栏显示 "Connected to simulator"**
- 各仪表数据随模拟器信号实时刷新

> 如果状态栏显示 "Disconnected" 或 "Mock mode"，说明网络连接失败，请检查模拟器是否在运行。

### 4.3 信号显示测试（需求 7.2）

连接成功后，观察仪表盘的数据显示是否符合以下预期：

| 信号 | 变化范围 | 变化规律 |
|:----|:--------|:--------|
| 速度 (speed) | 0 ~ 140 km/h | 正弦波动，周期约 10 秒，中心值 60 |
| 转速 (rpm) | 0 ~ 5600 | 随速度联动，`rpm = speed x 40` |
| 油量 (fuel) | 60L 缓慢下降至接近 0 | 每 100ms 减少 0.001L |
| 水温 (temp) | 80 ~ 100 C | 随机游走，每步变化不超过 5C |

**验证方法**：

1. 速度表指针应平滑摆动，范围覆盖 0~140 刻度
2. 转速表指针跟随速度同步变化
3. 油量指针（或数字）随时间缓慢下降
4. 水温指针在小范围内随机抖动

### 4.4 控制测试（需求 7.3）

通过仪表盘的菜单栏操作，验证控制指令的收发与响应：

| 操作 | 预期效果 |
|:----|:--------|
| 菜单 -> 信号控制 -> 左转 | 左转向灯亮起（绿色箭头闪烁） |
| 菜单 -> 信号控制 -> 右转 | 右转向灯亮起（绿色箭头闪烁） |
| 菜单 -> 信号控制 -> 双闪 | 左右转向灯同时闪烁 |
| 菜单 -> 信号控制 -> 关闭 | 所有转向灯熄灭 |
| 等待 speed > 120 | **ENG 报警灯亮起**，同时播放超速报警音效 |
| speed 回落至 120 以下 | **ENG 报警灯熄灭**，音效停止 |
| 等待 fuel < 15 | **FUEL 报警灯亮起**，同时播放低油量报警音效 |

> speed > 120 时模拟器自动将 `speedFault` 置 1，speed 回落后自动清零。油量从 60L 开始缓慢下降，约需等待数十秒可降至 15L 以下。

### 4.5 随机故障测试（需求 7.4）

保持仪表盘运行并连接模拟器，在 10~30 秒内观察：

- 随机故障灯（ABS、AIRBAG、BATTERY、BRAKE 之一）会**随机亮起**
- 亮起持续 **5~15 秒** 后自动熄灭
- 每次只有一个故障亮起，熄灭后可能触发另一个
- 故障灯亮起时仪表盘上有对应的指示灯或文字提示

模拟器的随机故障触发概率为 3%/秒（每 tick 约 0.3%），持续观察即可看到效果。

### 4.6 报警音效测试（需求 7.5）

| 触发条件 | 报警灯 | 音效文件 | 预期行为 |
|:--------|:------|:--------|:--------|
| 速度 > 120 km/h | ENG 灯亮 | `engine.wav` | 连续播放超速报警音，速度回落停止 |
| 油量 < 15 L | FUEL 灯亮 | `fuel.wav` | 连续播放低油量报警音，加油后停止 |
| 水温 > 100 C | TEMP 灯亮 | `temp.wav` | 连续播放高温报警音，温度回落停止 |

**音量调节测试**：

- 菜单 -> 音量控制 -> 静音：所有报警音立即停止
- 菜单 -> 音量控制 -> 取消静音：报警音恢复
- 菜单 -> 音量控制 -> 音量 +/-：报警音量增大或减小

> 音效文件需位于仪表盘可执行文件的同级目录或资源路径下。若无声音输出，请检查是否已静音或音效文件是否缺失。

### 4.7 Mock 模式（无需模拟器）

在没有模拟器或不需要联调时，仪表盘可以独立运行：

```cmd
cd work\dashboard
set PATH=D:\QT\Qt5.3.2\5.3\mingw482_32\bin;D:\QT\Qt5.3.2\Tools\mingw482_32\bin;%PATH%
release\dashboard.exe --mock
```

**预期结果**：

- 仪表盘窗口正常显示，窗口标题显示"汽车仪表盘 - Mock 模式"
- 状态栏显示 "Mock mode"
- 所有仪表数据由内置的 MockDataSource 生成，数据变化规律与模拟器基本一致
- 所有 UI 功能和报警逻辑均正常工作

Mock 模式适用于演示、UI 调试或模拟器不可用时的开发测试。

## 5. 命令行参数

| 参数 | 说明 |
|:----|:-----|
| `--mock` | Mock 模式。使用内置信号发生器，无需连接模拟器，独立运行 |
| （无参数）| 网络模式。默认连接 `127.0.0.1:5000` 的模拟器，需先启动模拟器 |

## 6. 常见问题

### Q: 启动后立即退出，窗口一闪而过

**原因**：缺少 `qt.conf` 文件或 PATH 未正确设置。Qt 5.3.2 找不到 `platforms/qwindows.dll` 插件。

**解决**：确保在 exe 同目录下有 `qt.conf`，内容如下：

```ini
[Paths]
Prefix = D:/QT/Qt5.3.2/5.3/mingw482_32
Plugins = plugins
```

### Q: 状态栏显示 "Disconnected" 或连接失败

**原因**：模拟器未启动或端口不匹配。

**解决**：
1. 先启动 `final_simulator.exe`，确认控制台显示 `listening on 0.0.0.0:5000`
2. 再启动 `dashboard.exe`（无参数模式）
3. 检查防火墙是否阻止了 5000 端口的 TCP 连接

### Q: 报警时没有声音

**原因**：
1. 音效文件（`engine.wav`、`fuel.wav`、`temp.wav`）缺失
2. 仪表盘处于静音状态
3. 多媒体模块编译有问题

**解决**：
1. 检查 exe 同级目录下是否存在 `.wav` 文件
2. 通过菜单 -> 音量控制 -> 取消静音
3. 确认 `.pro` 文件中包含 `QT += multimedia`

### Q: 双闪与转向灯冲突

仪表盘的逻辑是：开启双闪时，左右转向灯同时闪烁；此时如果操作左转或右转，双闪优先，转向灯指令将被忽略。关闭双闪后转向灯恢复正常控制。

### Q: 构建时报 "cannot find -lQt5Multimedia" 或类似链接错误

**原因**：qmake 未启用 multimedia 模块。

**解决**：确认 `dashboard.pro` 中有 `QT += multimedia`，然后重新执行 qmake：

```cmd
qmake dashboard.pro -spec win32-g++
mingw32-make clean
mingw32-make -j4
```

# final_simulator — 汽车信号模拟器（P6 服务端）

> 角色：**P6 服务端开发工程师**
> 对应需求文档：Section 2.1（信号模拟器）、Section 3（数据通信协议）、Section 8（JSON字段）
> 对应分工表：P6 — 服务端开发（模拟器）

## 1. 交付物清单

| 文件 | 说明 | 状态 |
|:----|:-----|:----:|
| `final_simulator.pro` | Qt 5 项目文件（core + network，console 应用） | ✅ |
| `simulator.h` | Simulator 类声明（TCP Server、多客户端、信号/故障状态） | ✅ |
| `simulator.cpp` | TCP 服务器 + 100ms JSON 广播 + 信号生成 + 故障系统 + 指令解析 | ✅ |
| `main.cpp` | 程序入口，`--port` 选项，控制台状态输出 | ✅ |
| `run.bat` | Windows 启动脚本（自动配置 Qt 路径） | ✅ |
| `release\final_simulator.exe` | 编译产物（39KB） | ✅ |
| `README.md` | 本文档 | ✅ |

## 2. 编译方法

环境：**Qt 5.3.2 + MinGW 4.8.2 (C++11)**（与 P5 dashboard 完全一致）

```cmd
cd work\final_simulator
set PATH=D:\2026Qt\QT\5.3\mingw482_32\bin;D:\2026Qt\QT\Tools\mingw482_32\bin;%PATH%
qmake final_simulator.pro -spec win32-g++
mingw32-make -j4
```

产物：`release\final_simulator.exe`（控制台应用，无 GUI）。

> 已验证：编译零错误零警告，`moc_simulator.cpp` 自动生成正常。

## 3. 运行方法

### 3.1 启动模拟器

方式一：双击 `run.bat`
方式二：命令行
```cmd
final_simulator.exe            REM 默认监听 5000 端口
final_simulator.exe --port 5001  REM 自定义端口
```

启动后控制台输出：
```
[sim] final_simulator listening on 0.0.0.0:5000 (waiting for dashboard clients)
[sim] final_simulator running. Press Ctrl+C to stop.
[sim] Dashboard clients should connect to 127.0.0.1:5000
```

### 3.2 启动仪表盘联调

**必须先启动模拟器**，然后在 dashboard 目录：
```cmd
cd ..\dashboard
dashboard.exe --connect
```

`--connect` 参数使 dashboard 进入网络模式（NetworkClient），连接 `127.0.0.1:5000`。
不加参数则为 Mock 模式（独立运行，不连模拟器）。

## 4. 功能实现对照（需求文档 2.1）

### 4.1 TCP 服务器（2.1.1）✅
- 监听端口 **5000**（`QHostAddress::Any`，支持多网卡）
- **支持多客户端**：`newConnection` 循环 accept，每客户端独立 `QTcpSocket`
- **每 100ms 广播一次 JSON**（`QTimer` 驱动 `onBroadcastTick`）
- 接收客户端控制指令并响应（`readyRead` + `canReadLine`）

### 4.2 信号生成（2.1.2）✅

| 信号 | 生成规则 | 实现 | 验证 |
|:-----|:--------|:-----|:----:|
| speed | 正弦波 60±80，周期10s，范围0~140 | `60 + 80*sin(2π*t/10000)`，钳位[0,140] | ✅ |
| rpm | speed×40 | `int(speed*40)` | ✅ |
| fuel | 初始60L，每100ms减0.001L，最小0 | 递减，钳位≥0 | ✅ |
| temp | 随机游走 80~100°C，每步±5°C | `qrand()%11 - 5`，钳位[80,100] | ✅ |
| turnSignal | 0/1/2 | 客户端指令设置 | ✅ |
| hazard | 0/1 | 客户端指令设置 | ✅ |

### 4.3 随机故障报警（2.1.3）✅

| 故障 | 触发 | 持续时间 | JSON字段 |
|:-----|:-----|:--------|:--------|
| absFault | 3%/秒随机 | 5~15秒自动恢复 | absFault |
| airbagFault | 3%/秒随机 | 5~15秒自动恢复 | airbagFault |
| batteryFault | 3%/秒随机 | 5~15秒自动恢复 | batteryFault |
| brakeFault | 3%/秒随机 | 5~15秒自动恢复 | brakeFault |
| speedFault | speed>120 自动触发 | 速度<120 自动消除 | speedFault |

> 随机故障按"3%/秒"在 **100ms tick 内折算为 0.3%/tick**（`qrand()%1000 < 3`），等价于每秒约 3%。
> 故障恢复用 `FaultRecovery` 结构记录 `recoverAtMs`，到点自动清零。

### 4.4 控制指令（2.1.4）✅

| 指令 | 格式 | 行为 |
|:-----|:-----|:-----|
| 转向灯 | `turnSignal:0/1/2\n` | 设置 m_turnSignal（钳位0~2） |
| 双闪 | `hazard:0/1\n` | 设置 m_hazard |
| 重置 | `reset\n` | 所有状态恢复初始值，立即推送一帧 |

> 指令按 `\n` 分行解析（`canReadLine`），忽略未知指令（前向兼容）。

### 4.5 JSON 数据格式（2.1.5）✅

每帧格式（紧凑、字段顺序与文档示例一致）：
```json
{"speed":122.3,"rpm":4892,"fuel":59.99,"temp":85.0,"turnSignal":0,"hazard":0,"absFault":0,"airbagFault":0,"batteryFault":0,"brakeFault":0,"speedFault":1}
```
- 每帧以 `\n` 结尾
- 数值精度：speed 1位小数、fuel 2位小数、temp 1位小数
- 编码：UTF-8（ASCII 子集）

## 5. 通信协议（需求文档 Section 3）

| 项 | 值 |
|:---|:---|
| 协议 | TCP/IP |
| 服务端地址 | 0.0.0.0:5000 |
| 客户端地址 | 127.0.0.1 |
| 消息分隔符 | `\n` |
| 方向1（服务端→客户端）| JSON 行，100ms 周期 |
| 方向2（客户端→服务端）| 纯文本指令行 |

与 P5 的 `networkclient.cpp` **完全对齐**（字段名、分隔符、指令格式一致）。

## 6. 联调验证记录（2026-06-16）

### 6.1 协议测试（独立 winsock 测试客户端）
连接模拟器后接收到的连续帧：
```
{"speed":122.3,"rpm":4892,"fuel":59.99,"temp":85.0,...,"speedFault":1}   ← speed>120 自动触发
发送: hazard:1
{"speed":125.5,...,"hazard":1,...}   ← 指令即时生效
```
**结论**：
- ✅ TCP 连接正常
- ✅ 100ms 周期推送稳定
- ✅ 速度正弦波动、rpm=speed×40、油量递减、水温随机游走
- ✅ speed>120 → speedFault=1 自动触发
- ✅ hazard:1 指令下一帧即生效（反馈延迟<100ms）

### 6.2 端到端联调（dashboard.exe --connect）
- 启动 `final_simulator.exe` → 监听 5000
- 启动 `dashboard.exe --connect` → **成功连接**
- 模拟器控制台日志记录到 dashboard 发来的真实指令：
  - `Command: turnSignal=1`
  - `Command: hazard=1`
  - `Command: reset (state cleared)`
- dashboard 窗口标题显示 `汽车仪表盘 - Socket 模式`（网络模式，非 Mock）

**联调通过标准**：仪表盘 UI 数据刷新与模拟器推送数值一致，指令收发正常。✅

## 7. 状态栏 / 日志输出

模拟器是控制台应用，所有状态输出到 stdout（前缀 `[sim]`），便于调试：
- 监听成功/失败
- 客户端连接/断开（含客户端数）
- 收到的控制指令
- 启动失败原因（端口占用等）

## 8. 与其他角色协作

| 角色 | 接口点 |
|:-----|:------|
| **P4 Tech Lead** | 架构审查、协议对齐（已完成对齐） |
| **P5 客户端** | `networkclient.cpp` ←→ 本模拟器（已联调通过） |
| **P7 QA** | 测试用例：启动模拟器→dashboard --connect，验证 Section 7 全部场景 |
| **P8 DevOps** | 本目录结构、`final_simulator.pro`、`run.bat`、README 即构建/运行交付物 |

## 9. 已知边界 / 设计取舍

- 故障"3%/秒"按 tick 折算为"0.3%/tick"（数学等价），非按真实秒计时——更稳定且符合常见模拟器实现（与 P5 的 MockDataSource 口径一致）。
- 水温源值游走范围 80~100（按需求 2.1.2），客户端再做 40~120 的钳位显示（按接口文档 2.3）。
- speedFault 是派生量（直接由 speed>120 决定），不参与随机故障系统，speed 回落自动清除。
- 模拟器无 GUI，纯控制台；关窗或 Ctrl+C 即停止。

# P5 客户端开发：汽车仪表盘程序

## TL;DR

> **快速概要**: 为 P5 角色（客户端开发工程师）创建完整的汽车仪表盘 Qt 5.3.2 程序，包含9个组件模块，支持**Mock模式（独立运行）** 和**网络模式（对接模拟器）** 双模式。
>
> **交付物**:
> - 完整可运行的 `dashboard.exe`（支持 Mock/Network 双模式）
> - 8个组件模块（speedometer, rpmgauge, fuelgauge, tempgauge, turnindicator, warninglights, warningsoundsystem, mainwindow）
> - 抽象数据接口 `IDataSource` + 两个实现：`NetworkClient`(TCP) + `MockDataSource`(本地模拟)
> - 接口文档 `接口文档.md`（含Mock占位标注）
> - 资源文件（图片 + 音效占位文件）
> - dashboard.pro 项目文件
>
> **预计工作量**: 大型（14次迭代提交）
> **开发环境**: Qt 5.3.2 + MinGW 4.8.2 (C++11) | Windows
> **关键路径**: 脚手架 → IDataSource+MockDataSource → NetworkClient → MainWindow → 各仪表 → 报警灯+音效 → 菜单集成 → 接口文档 → 打磨

---

## Context

### 原始需求
用户是分工表中的P5（客户端开发工程师），需要一人完成整个仪表盘客户端程序。

### 已确认决策
- ✅ **速度表范围**: 保持0-240km/h（按需求文档）
- ✅ **缺失资源**: 先用QPainter生成占位资源，留好接口后续替换
- ✅ **文件名**: `air bag.png` → `airbag.png`
- ✅ **窗口**: 固定大小 `setFixedSize(900,650)`
- ✅ **音频**: 使用 `QSound`（Qt5.3.2 Multimedia模块）
- ✅ **连接状态**: 添加状态指示器（满足测试用例"重连提示"要求）
- ✅ **reset指令**: 格式 `reset\n`
- ✅ **图片源路径**: `图片资源/images/images/` → 复制到 `work/dashboard/images/`

### Metis Review
**识别差距**（已处理）:
- 速度表范围不一致 → 已确认保持0-240
- 缺失资源 → 占位策略已确认
- 文件名空格 → 重命名已确认
- 窗口缩放 → 固定大小已确认
- TCP 消息分割需实现 `\n` 缓冲解析
- 所有仪表值需做钳位处理（speed≥0, fuel≥0, temp钳位）
- 故障JSON字段需做存在性检查
- 源文件必须 ASCII 编码（无 BOM、无中文注释）
- 单音播放：停止当前再播放下一个

---

## Work Objectives

### 核心目标
实现完整的汽车仪表盘 TCP 客户端程序，通过 JSON-over-TCP 接收模拟器数据并实时显示。

### 具体交付物
- `work/dashboard/dashboard.pro` - Qt项目文件
- `work/dashboard/main.cpp` - 程序入口（支持命令行参数选模式）
- `work/dashboard/idatasource.h` - **抽象数据接口**（Mock/Network统一接口）
- `work/dashboard/mockdatasource.h/cpp` - **Mock数据源**（本地模拟信号生成）
- `work/dashboard/networkclient.h/cpp` - TCP客户端（网络模式）
- `work/dashboard/mainwindow.h/cpp` - 主窗口
- `work/dashboard/speedometer.h/cpp` - 速度表
- `work/dashboard/rpmgauge.h/cpp` - 转速表
- `work/dashboard/fuelgauge.h/cpp` - 油量表
- `work/dashboard/tempgauge.h/cpp` - 水温表
- `work/dashboard/turnindicator.h/cpp` - 转向灯
- `work/dashboard/warninglights.h/cpp` - 报警灯组
- `work/dashboard/warningsoundsystem.h/cpp` - 报警音效
- `工作接口文档.md` - **接口文档**（含Mock占位标注，方便联调）
- `work/dashboard/images/` - 图片资源目录
- `work/dashboard/sounds/` - 音效资源目录
- `work/dashboard/run.bat` - 运行脚本
- `work/dashboard/qt.conf` - Qt插件路径配置

### 完成定义
- [ ] `qmake -spec win32-g++ dashboard.pro` 成功（退出码0）
- [ ] `mingw32-make -j4` 成功（退出码0）
- [ ] `debug/dashboard.exe` 生成成功
- [ ] 与模拟器联动：仪表数据实时更新
- [ ] 所有9个组件功能正常工作
- [ ] 14次git提交记录，中文信息

### Must Have
- **IDataSource 抽象接口**：NetworkClient 和 MockDataSource 统一继承，MainWindow 面向接口编程
- **Mock模式**：无需模拟器可独立运行，内建速度正弦波、油量递减、水温随机游走、随机故障
- **网络模式**：TCP连接 `127.0.0.1:5000`，自动重连（每3秒），JSON接收 `\n` 分割
- 命令行参数选择模式：`dashboard.exe --mock`（默认Mock），`dashboard.exe --connect`（网络模式）
- 所有表盘QPainter自绘（速度0-240，转速0-8000）
- 油量<15L→FUEL报警，水温>95°C→TEMP报警
- 速度>120→ENG报警（超速）
- 转向灯闪烁（500ms周期）
- 8个报警灯图标显示
- QSound单音播放+静音+音量控制
- 3个菜单栏：信号控制/报警测试/音效控制
- 源文件ASCII编码，无BOM
- **接口文档**：记录所有接口定义和Mock占位说明

### Must NOT Have（边界控制）
- 不添加需求外的仪表或组件
- 不做设置/持久化系统
- 不做单元测试（P7负责）
- 不做主题/皮肤切换
- 不做报警优先级队列（简单替换即可）
- 不做指数退避重连（固定3秒）
- 不做高DPI适配
- 不做录屏/回放功能
- 源文件不含中文注释
- **MockDataSource不代替P6的模拟器**——只用来前端自测，最终联调以NetworkClient对接P6为准

---

## Verification Strategy

### 测试决策
- **基础设施**: 无测试框架
- **自动化测试**: 无（P7负责测试）
- **Agent验证**: 每次构建后运行exe并用视觉模型截图验证

### QA策略
每个任务完成后，Agent执行以下验证：
1. **编译验证**: qmake + mingw32-make 退出码0
2. **运行验证**: 启动exe，确认窗口正常显示
3. **功能验证**: 用视觉模型截图对比预期效果
4. **证据保存**: 截图/日志保存到 `.omo/evidence/`

---

## Execution Strategy

### 迭代提交计划（14次）

```
提交 1:  项目脚手架 + 资源目录 + git初始化
提交 2:  IDataSource抽象接口 + MockDataSource 本地数据源
提交 3:  NetworkClient TCP通信模块（网络数据源）
提交 4:  MainWindow主窗口框架（支持Mock/Network双模式）
提交 5:  Speedometer 速度表
提交 6:  RpmGauge 转速表
提交 7:  FuelGauge 油量表 + TempGauge 水温表
提交 8:  TurnIndicator 转向灯
提交 9:  WarningLights 报警灯组
提交 10: WarningSoundSystem 报警音效
提交 11: 菜单栏集成 + 全组件联调
提交 12: 编写接口文档（含Mock占位标注）
提交 13: 联调测试与Bug修复
提交 14: 最终打磨 + qt.conf + run.bat
```

### 依赖矩阵
- 提交1（脚手架）: 无依赖
- 提交2（IDataSource+Mock）: 依赖提交1（需要.pro和项目结构）
- 提交3（NetworkClient）: 依赖提交2（实现IDataSource接口）
- 提交4（MainWindow）: 依赖提交2（面向IDataSource编程，无需等NetworkClient）
- 提交5-10（各仪表）: 依赖提交4（需要MainWindow提供布局区域）——**但可用Mock模式独立测试**
- 提交11（集成）: 依赖提交5-10全部就绪
- 提交12（接口文档）: 依赖提交11（接口已稳定）
- 提交13（联调）: 依赖提交11+12
- 提交14（打磨）: 依赖提交13

---

## TODOs

> 每个TODO = 一次git提交。请严格按顺序执行。
> 源文件使用ASCII编码（`[System.IO.File]::WriteAllText`），无BOM无中文注释。
> 每次提交后必须 `mingw32-make -j4` 验证编译通过。
> **重要**: Qt 5.3.2 的 qmake 无法处理含 Unicode 的路径，所有操作在 `work/dashboard/` 下进行。

- [x] 1. 项目脚手架搭建 + 资源目录初始化

  **做什么**:
  1. 创建 `work/` 和 `work/dashboard/` 目录
  2. 创建 `work/dashboard/dashboard.pro`（QT += core gui widgets network multimedia）
  3. 创建 `work/dashboard/main.cpp`（基本QApplication骨架，创建MainWindow并显示）
  4. 创建 `work/dashboard/images/` 和 `work/dashboard/sounds/` 目录
  5. 从 `图片资源/images/images/` 复制图片到 `work/dashboard/images/`，同时：
     - `air bag.png` → `airbag.png`（重命名）
     - 缺失的 `background.png` 和 `oil.png` 用QPainter生成占位图
  6. 创建占位wav文件（生成8个静音wav到 `sounds/`）
  7. 创建 `work/dashboard/qt.conf`
  8. 初始化git仓库，首次提交

  **项目文件结构**（创建后）:
  ```
  work/dashboard/
  ├── dashboard.pro
  ├── main.cpp
  ├── images/
  │   ├── background.png (占位)
  │   ├── speed.png
  │   ├── abs.png
  │   ├── airbag.png (原名 air bag.png)
  │   ├── battery_fault.png
  │   ├── brake.png
  │   ├── fuel.jpeg
  │   ├── temp.jpg
  │   ├── left.png
  │   ├── right.png
  │   └── oil.png (占位)
  ├── sounds/
  │   ├── engine.wav (静音占位)
  │   ├── oil.wav
  │   ├── battery.wav
  │   ├── temp.wav
  │   ├── fuel.wav
  │   ├── brake.wav
  │   ├── airbag.wav
  │   └── abs.wav
  └── qt.conf
  ```

  **dashboard.pro** 内容概要:
  ```qmake
  QT       += core gui widgets network multimedia
  CONFIG   += c++11
  TARGET    = dashboard
  TEMPLATE  = app
  SOURCES  += main.cpp
  HEADERS  +=
  INCLUDEPATH += .
  ```

  **main.cpp** 内容概要:
  ```cpp
  #include <QApplication>
  // 后续会包含 MainWindow
  int main(int argc, char *argv[])
  {
      QApplication app(argc, argv);
      // MainWindow w;
      // w.show();
      return app.exec();
  }
  ```

  **必须不做**:
  - 不要添加任何组件实现（只是骨架）
  - 不要添加中文注释到源文件

  **验证**:
  ```bash
  cd work/dashboard
  $env:PATH = "D:\\QT\\Qt5.3.2\\5.3\\mingw482_32\\bin;D:\\QT\\Qt5.3.2\\Tools\\mingw482_32\\bin;$env:PATH"
  qmake dashboard.pro -spec win32-g++
  mingw32-make -j4
  ```
  预期: 编译成功，生成 `debug/dashboard.exe`

  **引用**:
  - 需求文档 Section 5.2（项目结构）
  - Qt技能: qt532-new-project.ps1 脚手架脚本
  - 注意: 使用 ASCII 编码创建文件（`[System.IO.File]::WriteAllText($path, $content, [System.Text.Encoding]::ASCII)`）

  **QA场景**:
  ```
  场景: 项目脚手架编译验证
    工具: Bash
    前置: work/dashboard 目录创建完成
    步骤:
      1. cd work/dashboard
      2. qmake dashboard.pro -spec win32-g++
      3. mingw32-make -j4
      4. 检查退出码和 debug/dashboard.exe 是否存在
    预期: 退出码0，dashboard.exe 约30KB+
    证据: .omo/evidence/task1-build.log

  场景: 资源目录检查
    工具: Bash
    前置: 资源复制完成
    步骤:
      1. Get-ChildItem images/ | Select-Object Name
      2. Get-ChildItem sounds/ | Select-Object Name
    预期: images/ 有 11 个文件(含占位)，sounds/ 有 8 个 wav
    证据: .omo/evidence/task1-resources.log
  ```

  **提交**: YES
  - 信息: `feat: 项目脚手架搭建与资源目录初始化`
  - 文件: `dashboard.pro, main.cpp, images/*, sounds/*, qt.conf`
  - 验证: `qmake + mingw32-make`

- [x] 2. 实现 IDataSource 抽象接口 + MockDataSource 本地数据源

  **做什么**:
  1. 创建 `idatasource.h` — 纯虚接口类（QObject子类）
  2. 创建 `mockdatasource.h/cpp` — Mock数据源实现

  **IDataSource 接口定义**:
  ```cpp
  class IDataSource : public QObject {
      Q_OBJECT
  public:
      // 控制指令（由菜单触发）
      virtual void sendTurnSignal(int state) = 0;   // 0=关, 1=左, 2=右
      virtual void sendHazard(bool on) = 0;
      virtual void reset() = 0;

  signals:
      // 数据信号（与NetworkClient旧信号完全一致，确保MainWindow不依赖具体实现）
      void speedUpdated(double speed);
      void rpmUpdated(int rpm);
      void fuelUpdated(double fuel);
      void tempUpdated(double temp);
      void turnSignalUpdated(int state);
      void hazardUpdated(bool on);
      void faultUpdated(int index, bool active);
      void connected();
      void disconnected();
      void statusMessage(const QString &msg);  // 状态栏信息
  };
  ```

  3. **MockDataSource 实现** — 在客户端内建信号模拟器，包含：
     - `QTimer` 每100ms触发一次更新
     - 速度: 正弦波 `60 + 80 * sin(2π * t / 10)`，范围0~140
     - 转速: `rpm = speed * 40`
     - 油量: 初始60L，每100ms减0.001L，最小0
     - 水温: 随机游走80~100°C，每步±5°C，钳位到40~120
     - 转向灯: 初始0，通过 `sendTurnSignal()` 改变
     - 双闪: 初始false，通过 `sendHazard()` 改变
     - 随机故障: absFault/airbagFault/batteryFault/brakeFault 各3%/秒概率
     - 超速故障: speed>120时自动触发 speedFault
     - 故障持续5~15秒后自动恢复
  4. 发射信号与IDataSource接口定义一致

  **MockDataSource 设计要点**:
  - 继承 `IDataSource`，实现所有纯虚函数
  - 开始时自动发射 `connected()` 信号（模拟"已连接"状态）
  - `sendTurnSignal()` 更新内部状态，下一周期通过信号发射
  - `reset()` 将所有值重置到初始状态
  - 故障触发后启动QTimer计时，到时自动恢复

  **必须不做**:
  - 不要在这里实现NetworkClient（那是下一个任务）
  - MockDataSource不覆盖P6的模拟器职责，仅用于前端自测

  **验证**:
  ```bash
  qmake dashboard.pro -spec win32-g++ && mingw32-make -j4
  ```
  预期: 退出码0

  **引用**:
  - 需求文档 Section 2.1.2-2.1.3（信号生成规则、随机故障）
  - 需求文档 Section 8.1（JSON字段说明）
  - Qt技能: references/qtcore.md（QTimer, QObject信号）

  **QA场景**:
  ```
  场景: 编译验证
    工具: Bash
    步骤: qmake + mingw32-make
    预期: 退出码0
    证据: .omo/evidence/task2-build.log

  场景: Mock信号测试
    工具: 简单的测试main（仅验证MockDataSource能发射信号）
    步骤: 在main.cpp中临时new MockDataSource，connect信号到lambda打印
    预期: 每100ms看到数据输出
    证据: .omo/evidence/task2-mock-test.log
  ```

  **提交**: YES
  - 信息: `feat: 实现IDataSource抽象接口和MockDataSource本地数据源`
  - 文件: `idatasource.h, mockdatasource.h, mockdatasource.cpp, dashboard.pro`
  - 验证: `qmake + mingw32-make`

- [x] 3. 实现 NetworkClient TCP通信模块（网络数据源）

  **做什么**:
  1. 创建 `networkclient.h` 和 `networkclient.cpp`
  2. 继承 `IDataSource` 接口，实现所有纯虚函数
  3. 实现 QTcpSocket 连接 `127.0.0.1:5000`
  4. 自动重连：断开后每3秒重试
  5. 连接超时：5秒超时检测
  6. JSON 接收：按 `\n` 分割消息，QJsonDocument 解析
  7. 对每个 JSON 字段做存在性检查（`value().isUndefined()`），缺失则用默认值
  8. 所有仪表值做钳位（speed≥0, fuel≥0, temp在40-120之间）
  9. 发送控制指令：`turnSignal:N\n`, `hazard:N\n`, `reset\n`
  10. 发射 IDataSource 定义的信号

  **必须不做**:
  - 不做指数退避（固定3秒重连）
  - 不做SSL/TLS（纯文本TCP）
  - 不做多线程

  **验证**: qmake + mingw32-make → 退出码0

  **引用**:
  - 需求文档 Section 2.2.10 + Section 3（通信协议）
  - Qt技能: references/qtnetwork.md, references/qtcore.md
  - 参考 Task 2 的 `idatasource.h` 接口定义

  **QA**:
  ```
  场景: 编译 | Bash | qmake+make | 退出0 | .omo/evidence/task3-build.log
  ```

  **提交**: YES | `feat: 实现NetworkClient网络通信模块（继承IDataSource）` | networkclient.h/cpp, .pro

- [x] 4. 实现 MainWindow 主窗口框架（支持Mock/Network双模式）

  **做什么**:
  1. 创建 `mainwindow.h` 和 `mainwindow.cpp`
  2. 窗口标题: "汽车仪表盘 - Socket 模式"
  3. 固定大小: `setFixedSize(900, 650)`
  4. 加载背景图: 先尝试 `images/background.png`，若失败用 `#c0c0c0`

  **核心设计: 面向接口编程**
  - MainWindow 接受 `IDataSource*` 指针，不关心具体实现
  - 所有信号连接通过 `IDataSource` 接口进行
  - 菜单的控制指令通过 `IDataSource::sendTurnSignal()` 等纯虚函数发送

  5. 布局架构：左右两个表盘 + 中间面板（油量/水温/转向灯/报警灯）
  6. 菜单栏创建但不填充完整功能（占位，后续提交填充）
  7. 状态栏显示连接状态（通过 `IDataSource::statusMessage` 信号）
  8. 重写 `paintEvent` 绘制深蓝黑渐变背景（若背景图加载失败）
  9. 更新 `main.cpp` 支持命令行参数选择数据源模式:
     ```cpp
     int main(int argc, char *argv[]) {
         QApplication app(argc, argv);
         IDataSource *dataSource;
         if (app.arguments().contains("--connect")) {
             dataSource = new NetworkClient();    // 网络模式
         } else {
             dataSource = new MockDataSource();   // Mock模式（默认）
         }
         MainWindow w(dataSource);
         w.show();
         return app.exec();
     }
     ```

  **布局结构**（900x650）:
  ```
  ┌──────────────────────────────────────────────────────┐  0
  │  菜单栏 (QMenuBar)                                    │  25
  ├────────────┬──────────────────────┬──────────────────┤
  │            │  [转向灯] ← →        │                  │
  │  速度表     │  油量表  水温表      │   转速表          │
  │            │  报警灯组            │                  │
  │  250px宽   │   居中 400px         │  250px宽        │
  │            │                      │                  │
  │   y: 80    │    y: 80             │   y: 80          │
  │            │                      │                  │
  └────────────┴──────────────────────┴──────────────────┘
  └──────────────────────────────────────────────────────┘  650
  ```

  **必须不做**:
  - 不实现菜单功能（只是UI框架，后续提交填充）
  - 不在paintEvent中绘制仪表（由子组件绘制）

  **验证**:
  - 编译通过
  - 运行后显示主窗口（Mock模式，无需模拟器）

  **引用**:
  - 需求文档 Section 2.2.1-2.2.2（窗口规格+布局）
  - 需求文档 Section 4（颜色主题）
  - Qt技能: references/qtwidgets.md（QMainWindow, QMenuBar, QStatusBar）
  - Task 2 的 `idatasource.h` 接口定义

  **QA场景**:
  ```
  场景: 编译验证 | qmake+make | 退出0 | .omo/evidence/task4-build.log

  场景: Mock模式运行
    工具: Bash
    步骤: dashboard.exe（默认Mock模式），截图
    预期: 900x650窗口，深蓝黑背景，状态栏显示"Mock模式-独立运行"
    证据: .omo/evidence/task4-mainwindow.png
  ```

  **提交**: YES
  - 信息: `feat: 实现MainWindow主窗口框架（支持Mock/Network双模式）`
  - 文件: `mainwindow.h, mainwindow.cpp, main.cpp, dashboard.pro`
  - 验证: `qmake + mingw32-make`

- [x] 5. 实现 Speedometer 速度表

  **做什么**:
  1. 创建 `speedometer.h` 和 `speedometer.cpp`
  2. 继承 QWidget，重写 `paintEvent`
  3. 显示范围: 0~240 km/h，刻度: 0,20,40,60,80,100,120,140,160,180,200,220,240
  4. 设置 `setFixedSize(250, 480)`，位于左侧
  5. QPainter自绘半圆弧表盘
  6. 颜色方案（严格按需求）:
     - 常规刻度颜色: `#00ffaa`（青绿色）
     - 高速区(≥120)刻度颜色: `#ff4400`（红色）
     - 指针颜色: `#ff2200`（红色）
     - 刻度数字颜色: `#00ff88`
     - 表盘背景: 深蓝黑渐变（透明不遮挡窗口背景）
  7. 内圈显示当前速度数值（Arial 36pt Bold）
  8. 平滑动画: `current += (target - current) * 0.1`
  9. 接收信号 `speedUpdated(double)` 更新目标值
  10. 使用 `QTimer` 驱动重绘（约30fps刷新率）

  **速度表绘制要点**:
  ```
  圆弧范围: 从 225°（左下方）到 315°（右下方），即跨越270°
  刻度位置: 每20km/h一个主刻度线（共13个）
  指针角度: (speed / 240) * 270° - 135°（偏移到从225°开始）
  高速区: ≥120km/h 的刻度线用红色，其余用青绿色
  ```

  **必须不做**:
  - 不做额外的动画效果（如辉光、阴影）
  - 不允许修改窗口布局

  **验证**:
  - 编译通过
  - 视觉验证：表盘圆弧、刻度清晰可见

  **引用**:
  - 需求文档 Section 2.2.3（速度表规格）
  - 需求文档 Section 4（颜色主题+字体）
  - Qt技能: references/qtgui.md（QPainter绘图）

  **QA场景**:
  ```
  场景: 编译验证
    工具: Bash
    步骤: qmake + mingw32-make
    预期: 退出码0

  场景: 视觉验证
    工具: 启动程序 + 截图
    步骤:
      1. 运行 dashboard.exe
      2. 截图左侧速度表
    预期: 半圆弧表盘，有刻度线和数字，指针在0位置
    证据: .omo/evidence/task5-speedometer.png
  ```

  **提交**: YES
  - 信息: `feat: 实现Speedometer速度表`
  - 文件: `speedometer.h, speedometer.cpp, dashboard.pro`
  - 验证: `qmake + mingw32-make`

- [x] 6. 实现 RpmGauge 转速表

  **做什么**:
  1. 创建 `rpmgauge.h` 和 `rpmgauge.cpp`
  2. 继承 QWidget，重写 `paintEvent`
  3. 显示范围: 0~8000 RPM，刻度: 0,1k,2k,3k,4k,5k,6k,7k,8k
  4. 设置 `setFixedSize(250, 480)`，位于右侧
  5. QPainter自绘半圆弧表盘（与速度表对称）
  6. 颜色:
     - 常规刻度: `#00ffaa`
     - 高速区(≥6000): `#ff4400`
     - 指针: `#ff2200`
     - 数字: `#00ff88`
  7. 接收信号 `rpmUpdated(int)`
  8. 平滑动画: `current += (target - current) * 0.1`
  9. 内圈显示当前转速数值

  **必须不做**:
  - 不做额外动画
  - 不做指针阴影

  **验证**:
  - 编译通过
  - 视觉验证：与速度表对称布局

  **引用**:
  - 需求文档 Section 2.2.4（转速表规格）
  - 可参考Task 4的速度表实现，镜像到右侧

  **QA场景**:
  ```
  场景: 编译验证
    工具: Bash
    步骤: qmake + mingw32-make
    预期: 退出码0

  场景: 视觉验证
    工具: 运行+截图
    步骤: dashboard.exe 截图
    预期: 右侧转速表与左侧速度表对称，布局平衡
    证据: .omo/evidence/task5-rpmgauge.png
  ```

  **提交**: YES
  - 信息: `feat: 实现RpmGauge转速表`
  - 文件: `rpmgauge.h, rpmgauge.cpp, dashboard.pro`
  - 验证: `qmake + mingw32-make`

- [x] 7. 实现 FuelGauge + TempGauge 油量表和水温表

  **做什么**:
  1. 创建 `fuelgauge.h/cpp` 和 `tempgauge.h/cpp`（或者合并为一个 `gauges.h/cpp` 带两个类）
  2. 两个小表盘，位于中间面板
  3. 油量表规格:
     - 显示范围: 0~60 L
     - 刻度: 0, 15, 30, 45, 60
     - 接收信号 `fuelUpdated(double)`
     - 低油量报警条件: fuel < 15L → 发射信号触发 Warning_Fuel
     - 平滑动画
  4. 水温表规格:
     - 显示范围: 40~120 °C
     - 刻度: 40, 60, 80, 100, 120
     - 接收信号 `tempUpdated(double)`
     - 高水温报警条件: temp > 95°C → 发射信号触发 Warning_Temp
     - 钳位到 40-120 范围
     - 平滑动画
  5. 两个表盘水平排列在中间区域

  **布局**:
  ```
  中间面板 (400x480):
  ┌──────────────────────────────────────┐
  │           [转向灯位置待后续]           │
  │                                      │
  │   ┌──────────┐  ┌──────────┐        │
  │   │ 油量表    │  │ 水温表   │        │
  │   │ 0-60L    │  │ 40-120°C │        │
  │   │ Fuel     │  │ Temp     │        │
  │   └──────────┘  └──────────┘        │
  │                                      │
  │           [报警灯位置待后续]           │
  └──────────────────────────────────────┘
  ```

  **必须不做**:
  - 不添加第三个仪表
  - 不过度设计表盘（保持简洁风格）

  **验证**:
  - 编译通过
  - 视觉验证：两个小表盘水平排列

  **引用**:
  - 需求文档 Section 2.2.5（油量表）
  - 需求文档 Section 2.2.6（水温表）
  - 需求文档 Section 2.2.8 报警条件

  **QA场景**:
  ```
  场景: 编译验证
    工具: Bash
    步骤: qmake + mingw32-make
    预期: 退出码0

  场景: 视觉验证
    工具: 运行+截图
    步骤: dashboard.exe 截图
    预期: 中间区域显示两个小表盘
    证据: .omo/evidence/task6-gauges.png
  ```

  **提交**: YES
  - 信息: `feat: 实现FuelGauge油量表和TempGauge水温表`
  - 文件: `fuelgauge.h, fuelgauge.cpp, tempgauge.h, tempgauge.cpp, dashboard.pro`
  - 验证: `qmake + mingw32-make`

- [x] 8. 实现 TurnIndicator 转向灯

  **做什么**:
  1. 创建 `turnindicator.h/cpp`
  2. 继承 QWidget，重写 `paintEvent`
  3. 左右两个三角形箭头（← →），位于中间面板顶部
  4. 接收信号确定状态: `turnSignalUpdated(int)`, `hazardUpdated(bool)`
  5. 闪烁动画: 500ms周期（250ms亮/250ms暗）
  6. 双闪激活时左右同时闪烁
  7. hazard 优先于 turnSignal

  **转向灯逻辑**:
  ```
  if (hazard):    左右同时闪烁
  else if (1):    左闪烁，右灭
  else if (2):    右闪烁，左灭
  else:           左右都灭
  ```

  **Must NOT**:
  - 不做连续调光
  - 不添加声音

  **验证**: qmake + mingw32-make → 退出码0

  **引用**: 需求文档 2.2.7

  **QA**:
  ```
  场景: 编译 | Bash | qmake+make | 退出0 | .omo/evidence/task7-build.log
  ```

  **提交**: YES | `feat: 实现TurnIndicator转向灯` | turnindicator.h/cpp, .pro

- [x] 9. 实现 WarningLights 报警灯组

  **做什么**:
  1. 创建 `warninglights.h/cpp`
  2. 8个报警灯水平排列在中间面板底部
  3. 报警灯列表:

  | 索引 | 标签 | 自动触发 | 图片 |
  |:----:|:----:|:--------:|:----:|
  | 0 | ENG | speedFault=1 | speed.png |
  | 1 | OIL | 仅手动 | oil.png(占位) |
  | 2 | BAT | batteryFault=1 | battery_fault.png |
  | 3 | TEMP | temp>95°C | temp.jpg |
  | 4 | FUEL | fuel<15L | fuel.jpeg |
  | 5 | BRAKE | brakeFault=1 | brake.png |
  | 6 | AIR | airbagFault=1 | airbag.png |
  | 7 | ABS | absFault=1 | abs.png |

  4. 接收 `faultUpdated(int,bool)` 信号更新
  5. 支持手动开关（通过菜单）
  6. 激活态: 图标正常+红色背景；未激活: 灰色半透明+暗色
  7. 图片加载失败显示文字标签替代

  **Must NOT**: 不添加第9个灯，不做图标动画

  **验证**: qmake + mingw32-make → 退出码0

  **引用**: 需求文档 2.2.8, 8.2

  **QA**:
  ```
  场景: 编译 | Bash | qmake+make | 退出0 | .omo/evidence/task8-build.log
  ```

  **提交**: YES | `feat: 实现WarningLights报警灯组` | warninglights.h/cpp, .pro

- [x] 10. 实现 WarningSoundSystem 报警音效系统

  **做什么**:
  1. 创建 `warningsoundsystem.h/cpp`
  2. 使用 QSound 播放 wav（QT += multimedia）
  3. 8个音效映射: engine,oil,battery,temp,fuel,brake,airbag,abs
  4. 单音播放: 同时只播一个，新触发替换当前
  5. 报警解除后循环完毕自动停止
  6. 静音控制: `setMuted(bool)`
  7. 音量控制: `setVolume(int)`, 默认80%
  8. 文件缺失: 不崩溃，输出警告，不播放
  9. 懒加载: 首次触发时创建 QSound 实例

  **QSound用法**:
  ```cpp
  QSound *s = new QSound("sounds/engine.wav", this);
  s->setLoops(1);
  s->play();
  // s->stop();
  ```

  **Must NOT**: 不做优先级队列，不用QMediaPlayer，不预创建实例

  **验证**: qmake + mingw32-make → 退出码0

  **引用**: 需求文档 2.2.9, Qt技能 qtmultimedia.md

  **QA**:
  ```
  场景: 编译 | Bash | qmake+make | 退出0 | .omo/evidence/task9-build.log
  ```

  **提交**: YES | `feat: 实现WarningSoundSystem报警音效系统` | warningsoundsystem.h/cpp, .pro

- [x] 11. 菜单栏集成 + 全组件联动

  **做什么**:
  1. 完善 MainWindow 的3个菜单功能

  **菜单1: [信号控制]**:
  - 左转: 发送 `turnSignal:1\n`
  - 右转: 发送 `turnSignal:2\n`
  - 关闭转向: 发送 `turnSignal:0\n`
  - 双闪开: 发送 `hazard:1\n`
  - 双闪关: 发送 `hazard:0\n`
  - 重置: 发送 `reset\n`
  - 互斥逻辑: 左转时取消右转，反之亦然

  **菜单2: [报警灯测试]**:
  - 8个可勾选菜单项: ENG/OIL/BAT/TEMP/FUEL/BRAKE/AIR/ABS
  - 勾选=手动开灯，取消=关灯（仅本地效果）

  **菜单3: [音效控制]**:
  - ☐ 静音勾选框
  - 音量选择: 0%/20%/40%/60%/80%/100%
  - 8个"测试音效"菜单项
  - 分隔线分隔控制项和测试项

  2. 组件信号连接:
  ```
  NetworkClient::speedUpdated → Speedometer
  NetworkClient::rpmUpdated   → RpmGauge
  NetworkClient::fuelUpdated  → FuelGauge
  NetworkClient::tempUpdated  → TempGauge
  NetworkClient::turnSignalUpdated → TurnIndicator
  NetworkClient::hazardUpdated → TurnIndicator
  NetworkClient::faultUpdated → WarningLights
  FuelGauge::lowFuel  → WarningLights (fault 4)
  TempGauge::highTemp → WarningLights (fault 3)
  WarningLights::faultOn  → WarningSoundSystem
  WarningLights::faultOff → WarningSoundSystem
  ```

  3. 状态栏: 连接/断开指示

  **Must NOT**: 不添加第4个菜单，菜单项用英文标签

  **验证**: qmake + mingw32-make → 退出码0

  **引用**: 需求文档 2.2.9, 2.2.10

  **QA**:
  ```
  场景: 编译 | Bash | qmake+make | 退出0 | .omo/evidence/task10-build.log
  场景: 菜单显示 | 运行+截图 | 3个菜单可展开 | .omo/evidence/task10-menu.png
  ```

  **提交**: YES | `feat: 菜单栏集成与全组件联动` | mainwindow.h/cpp, .pro

- [x] 12. 编写接口文档（含Mock占位标注）

  **做什么**:
  1. 在项目根目录创建 `工作接口文档.md`
  2. 按照前后端接口规范编写完整文档
  3. 明确标注哪些接口当前使用Mock数据

  **接口文档结构**:
  ```markdown
  # 汽车仪表盘系统 — 前后端接口文档

  ## 1. 接口总览

  | 方向 | 接口 | 频率 | 当前状态 |
  |:----:|:----|:----:|:--------:|
  | 模拟器→仪表盘 | JSON数据推送 | 100ms | ✅ Mock可用 |
  | 仪表盘→模拟器 | 控制指令 | 按需 | ✅ Mock可用 |

  ## 2. 模拟器 → 仪表盘（数据推送）

  ### 2.1 TCP连接
  - **协议**: TCP/IP
  - **地址**: 127.0.0.1:5000
  - **消息分隔**: `\n` 换行符
  - **当前状态**: 🔶 Mock已实现，NetworkClient待联调

  ### 2.2 JSON数据格式
  | 字段 | 类型 | 范围 | 说明 | Mock实现 |
  |:----|:----|:----|:------|:--------:|
  | speed | double | 0~140 | 速度km/h，保留1位 | ✅ 正弦波60±80 |
  | rpm | int | 0~8000 | 转速 = speed×40 | ✅ 自动计算 |
  | fuel | double | 0~60 | 油量L，保留2位 | ✅ 递减(0.001L/100ms) |
  | temp | double | 40~120 | 水温°C，保留1位 | ✅ 随机游走80~100±5 |
  | turnSignal | int | 0/1/2 | 0=关1=左2=右 | ✅ 菜单控制 |
  | hazard | int | 0/1 | 0=关1=开 | ✅ 菜单控制 |
  | absFault | int | 0/1 | ABS故障 | ✅ 3%/秒随机 |
  | airbagFault | int | 0/1 | 安全气囊故障 | ✅ 3%/秒随机 |
  | batteryFault | int | 0/1 | 电瓶故障 | ✅ 3%/秒随机 |
  | brakeFault | int | 0/1 | 刹车故障 | ✅ 3%/秒随机 |
  | speedFault | int | 0/1 | 超速报警 | ✅ speed>120自动 |

  ### 2.3 JSON示例
  ```json
  {
    "speed": 98.5,
    "rpm": 3940,
    "fuel": 59.45,
    "temp": 88.3,
    "turnSignal": 1,
    "hazard": 0,
    "absFault": 0,
    "airbagFault": 0,
    "batteryFault": 0,
    "brakeFault": 0,
    "speedFault": 0
  }
  ```

  ## 3. 仪表盘 → 模拟器（控制指令）

  | 指令 | 格式 | 说明 | Mock实现 |
  |:----|:-----|:-----|:--------:|
  | 左转 | `turnSignal:1\n` | 打开左转向灯 | ✅ 内部状态切换 |
  | 右转 | `turnSignal:2\n` | 打开右转向灯 | ✅ 内部状态切换 |
  | 关转向 | `turnSignal:0\n` | 关闭转向灯 | ✅ 内部状态切换 |
  | 双闪开 | `hazard:1\n` | 打开双闪 | ✅ 内部状态切换 |
  | 双闪关 | `hazard:0\n` | 关闭双闪 | ✅ 内部状态切换 |
  | 重置 | `reset\n` | 重置所有状态 | ✅ 恢复初始值 |

  ## 4. 客户端内部接口（IDataSource）

  ### 4.1 抽象接口定义
  - 定义文件: `idatasource.h`
  - 两个实现: `MockDataSource`(当前默认), `NetworkClient`(联调时切换)
  - 切换方式: `dashboard.exe --mock`（Mock模式，默认），`dashboard.exe --connect`（网络模式）

  ### 4.2 信号列表
  - `speedUpdated(double)` / `rpmUpdated(int)` / `fuelUpdated(double)` / `tempUpdated(double)`
  - `turnSignalUpdated(int)` / `hazardUpdated(bool)`
  - `faultUpdated(int index, bool active)`
  - `connected()` / `disconnected()` / `statusMessage(const QString&)`

  ## 5. Mock说明

  - **当前状态**: 默认使用MockDataSource，无需启动模拟器即可独立运行
  - **Mock信号**: 完全模拟P6模拟器的数据生成规则
  - **转为联调**: 启动模拟器后，用 `--connect` 参数切换为NetworkClient
  - **Mock未覆盖**: 所有接口均已覆盖Mock，无遗漏
  ```

  **必须不做**:
  - 文档中不要写与实际实现不一致的内容
  - 不添加不存在的数据字段

  **验证**: 文件保存，格式正确

  **QA**:
  ```
  场景: 文档完整性 | 人工检查 | 所有接口都有Mock状态标注 | .omo/evidence/task12-doc-check.log
  ```

  **提交**: YES | `docs: 编写前后端接口文档（含Mock占位标注）` | `工作接口文档.md`

- [x] 13. 联调测试与Bug修复

  **做什么**:
  1. 启动P6模拟器（或自行启动测试服务），用 `--connect` 模式连接
  2. 验证数据流: 模拟器→NetworkClient→各组件更新
  3. 边界测试: speed=0/140/240, fuel=0/60, temp=40/120
  4. 故障测试: 所有fault位设为1检验报警灯
  5. 菜单操作测试: 所有菜单项点击验证
  6. 测试Mock→Network无缝切换
  7. 修复发现的Bug

  **验证**: `dashboard.exe --connect` 正常连接模拟器，数据更新

  **QA**:
  ```
  场景: 全链路 | Bash(模拟器+仪表盘--connect) | 数据跟随变化 | .omo/evidence/task13-integration.png
  ```

  **提交**: YES | `fix: 联调测试与Bug修复` | 修改过的文件

- [x] 14. 最终打磨 + 配置文件

  **做什么**:
  1. 创建 `run.bat`:
  ```bat
  @echo off
  set PATH=D:\QT\Qt5.3.2\5.3\mingw482_32\bin;D:\QT\Qt5.3.2\Tools\mingw482_32\bin;%%PATH%%
  start "" "%%~dp0debug\dashboard.exe"
  ```
  2. 确认 qt.conf 存在且正确
  3. 代码清理: 去除未使用变量/函数
  4. 确认 .pro 包含所有 HEADERS/SOURCES
  5. 最终编译验证

  **验证**: 全新 clone → qmake → make → run

  **QA**:
  ```
  场景: 最终编译 | Bash | qmake+make | 退出0 | .omo/evidence/task14-build.log
  场景: 运行 | Bash(run.bat) | 窗口正常 | .omo/evidence/task14-final.png
  ```

  **提交**: YES | `chore: 最终打磨与配置文件完善` | run.bat, qt.conf, .pro, 清理过的文件

---

## 最终验证

> 以下4个验证步骤在全部任务完成后依次执行。

- [x] F1. **编译验证**
  从干净状态重建:
  ```bash
  cd work/dashboard
  $env:PATH = "D:\\QT\\Qt5.3.2\\5.3\\mingw482_32\\bin;D:\\QT\\Qt5.3.2\\Tools\\mingw482_32\\bin;$env:PATH"
  qmake dashboard.pro -spec win32-g++
  mingw32-make distclean 2>$null; mingw32-make -j4
  ```
  预期: 退出码0，debug/dashboard.exe 存在

- [x] F2. **运行验证**
  1. 运行 dashboard.exe（需 qt.conf 或 PATH 设置正确）
  2. 截图确认窗口显示
  3. 检查窗口标题、大小、布局

- [x] F3. **视觉QA验证**
  使用视觉模型检查:
  1. 速度表刻度完整，半圆弧清晰
  2. 转速表与速度表对称
  3. 油量/水温表显示正常
  4. 转向灯箭头可识别
  5. 报警灯图标加载正常
  6. 整体配色符合需求（深蓝黑背景，霓虹绿/红点缀）

- [x] F4. **git历史验证**
  ```bash
  cd work/dashboard
  git log --oneline
  ```
  预期: 14次提交，中文信息，渐进式开发

---

## 提交策略

| 序号 | 提交信息 | 验证命令 |
|:----:|---------|---------|
| 1 | feat: 项目脚手架搭建与资源目录初始化 | qmake + mingw32-make |
| 2 | feat: 实现IDataSource抽象接口和MockDataSource本地数据源 | qmake + mingw32-make |
| 3 | feat: 实现NetworkClient网络通信模块（继承IDataSource） | qmake + mingw32-make |
| 4 | feat: 实现MainWindow主窗口框架（支持Mock/Network双模式） | qmake + mingw32-make |
| 5 | feat: 实现Speedometer速度表 | qmake + mingw32-make |
| 6 | feat: 实现RpmGauge转速表 | qmake + mingw32-make |
| 7 | feat: 实现FuelGauge油量表和TempGauge水温表 | qmake + mingw32-make |
| 8 | feat: 实现TurnIndicator转向灯 | qmake + mingw32-make |
| 9 | feat: 实现WarningLights报警灯组 | qmake + mingw32-make |
| 10 | feat: 实现WarningSoundSystem报警音效系统 | qmake + mingw32-make |
| 11 | feat: 菜单栏集成与全组件联动 | qmake + mingw32-make |
| 12 | docs: 编写前后端接口文档（含Mock占位标注） | - |
| 13 | fix: 联调测试与Bug修复 | qmake + mingw32-make |
| 14 | chore: 最终打磨与配置文件完善 | qmake + mingw32-make |

---

## 成功标准

### 验证命令
```bash
cd work/dashboard
qmake -spec win32-g++ dashboard.pro  # 退出码0
mingw32-make -j4                      # 退出码0，生成debug/dashboard.exe
```

### 最终检查清单
- [ ] dashboard.exe 编译成功
- [ ] 与模拟器正常通信
- [ ] 所有仪表显示正常
- [ ] 报警灯条件触发正常
- [ ] 声音播放正常
- [ ] 菜单操作正常
- [ ] 窗口布局符合需求
- [ ] 14次git提交记录完整

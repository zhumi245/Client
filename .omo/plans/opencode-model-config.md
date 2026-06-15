# OpenCode 模型配置更新 — 替换高优先级代理为 Qwen3.7 Plus

## TL;DR

> **Quick Summary**: 将 `oh-my-openagent.json` 中高优先级代理（Oracle、Metis、Momus、Prometheus）和分类（ultrabrain、deep、artistry）的模型从 `opencode-go/deepseek-v4-flash` 替换为 `opencode-go/qwen3.7-plus`，其余保持原样。
>
> **Deliverables**:
> - 编辑 `C:\Users\林舒乐\.config\opencode\oh-my-openagent.json` 中的 7 处模型 ID
>
> **Estimated Effort**: Quick（单文件编辑）
> **Parallel Execution**: NO - 顺序执行
> **Critical Path**: 读取 → 逐处替换 → 验证

---

## Context

### Original Request
用户要求将 OpenCode 中高优先级代理的模型从 deepseek-v4-flash 替换为 Qwen3.7 Plus。

### 模型信息（来自 OpenCode 官方文档）
- **Qwen3.7 Plus 模型 ID**: `opencode-go/qwen3.7-plus`
- **API 格式**: Anthropic 兼容（`@ai-sdk/anthropic`）
- **价格**: 输入 $0.40 / 1M tokens，输出 $1.60 / 1M tokens
- **对比 Flash**: Flash 价格为 $0.14 / $0.28，但高优先级代理（规划、审查、深度推理）值得用更强模型

### 判断依据
| 优先级 | 配置项 | 替换？ | 理由 |
|--------|--------|--------|------|
| P0 | `agents.oracle` | ✅ | 高 IQ 推理/架构决策，调用少但每次关键 |
| P1 | `agents.metis` | ✅ | 规划前分析，需要发现隐藏盲点 |
| P1 | `agents.momus` | ✅ | 计划审查，需要极其挑剔的眼光 |
| P1 | `agents.prometheus` | ✅ | 规划质量决定全局效率 |
| P2 | `categories.ultrabrain` | ✅ | 专用于真正困难的逻辑问题 |
| P2 | `categories.deep` | ✅ | 深度研究任务 |
| P2 | `categories.artistry` | ✅ | 创造性问题解决 |
| — | 其余 8 项 | ❌ | 高频/简单任务，flash 更经济高效 |

---

## Work Objectives

### Core Objective
替换 `oh-my-openagent.json` 中 7 个高优先级配置项的模型 ID。

### Concrete Deliverables
- 编辑后的 `C:\Users\林舒乐\.config\opencode\oh-my-openagent.json`

### Definition of Done
- [ ] 运行 `Get-Content "$env:USERPROFILE\.config\opencode\oh-my-openagent.json"` 确认 7 处替换正确

### Must Have
- agents.oracle 的 model 改为 `opencode-go/qwen3.7-plus`
- agents.metis 的 model 改为 `opencode-go/qwen3.7-plus`
- agents.momus 的 model 改为 `opencode-go/qwen3.7-plus`
- agents.prometheus 的 model 改为 `opencode-go/qwen3.7-plus`
- categories.ultrabrain 的 model 改为 `opencode-go/qwen3.7-plus`
- categories.deep 的 model 改为 `opencode-go/qwen3.7-plus`
- categories.artistry 的 model 改为 `opencode-go/qwen3.7-plus`
- 其余所有代理和分类保持 `opencode-go/deepseek-v4-flash` 不变

### Must NOT Have (Guardrails)
- 不要改动 `multimodal-looker`（它用的是 `mimo-v2.5`）
- 不要改动 `explore`、`librarian`、`sisyphus`、`hephaestus`、`sisyphus-junior` 等低优先级代理
- 不要改动 `visual-engineering`、`quick`、`unspecified-low`、`unspecified-high`、`writing` 分类
- 不要修改 `opencode.json` 或 `tui.json`

---

## Verification Strategy

### Test Decision
- **Infrastructure exists**: NO（无测试框架）
- **Automated tests**: None
- **Agent-Executed QA**: 执行后用 `Get-Content` 读取文件验证内容

### QA Policy
任务完成后，执行代理必须读取文件并逐项确认 7 处替换正确，且未被改动的项保持原值。

---

## Execution Strategy

单任务、单文件、顺序执行。

---

## TODOs

- [x] 1. 编辑 oh-my-openagent.json — 替换 7 个配置项的模型 ID

  **What to do**:
  - 读取 `C:\Users\林舒乐\.config\opencode\oh-my-openagent.json`
  - 逐处将以下配置项的 `"model"` 从 `"opencode-go/deepseek-v4-flash"` 改为 `"opencode-go/qwen3.7-plus"`：
    1. `agents.oracle.model`
    2. `agents.metis.model`
    3. `agents.momus.model`
    4. `agents.prometheus.model`
    5. `categories.ultrabrain.model`
    6. `categories.deep.model`
    7. `categories.artistry.model`
  - 每个替换用 Edit 工具操作，确保 `oldString` 包含足够上下文避免误匹配
  - 确认所有非替换项未被触及

  **Must NOT do**:
  - 不要改动 agents 下的 sisyphus、hephaestus、librarian、explore、multimodal-looker、sisyphus-junior、atlas
  - 不要改动 categories 下的 visual-engineering、quick、unspecified-low、unspecified-high、writing
  - 不要改动 `opencode.json` 或 `tui.json`

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: 单文件编辑，7 处简单替换，属于快速修改任务

  **Parallelization**:
  - **Can Run In Parallel**: NO（单文件）

  **References**:
  - `C:\Users\林舒乐\.config\opencode\oh-my-openagent.json` - 目标配置文件
  - `C:\Users\林舒乐\.config\opencode\opencode.json` - 不要改动
  - `https://opencode.ai/docs/zh-cn/go/` - 官方文档确认模型 ID 为 `opencode-go/qwen3.7-plus`

  **Acceptance Criteria**:

  **QA Scenarios**:

  ```
  Scenario: 验证配置文件修改正确
    Tool: Bash (Get-Content)
    Preconditions: 配置文件已编辑保存
    Steps:
      1. 运行 `Get-Content "$env:USERPROFILE\.config\opencode\oh-my-openagent.json"`
      2. 搜索 "qwen3.7-plus" 出现次数应为 7 次
      3. 逐项确认以下行存在：
         - `"oracle":` 下方 → `"model": "opencode-go/qwen3.7-plus"`
         - `"metis":` 下方 → `"model": "opencode-go/qwen3.7-plus"`
         - `"momus":` 下方 → `"model": "opencode-go/qwen3.7-plus"`
         - `"prometheus":` 下方 → `"model": "opencode-go/qwen3.7-plus"`
         - `"ultrabrain":` 下方 → `"model": "opencode-go/qwen3.7-plus"`
         - `"deep":` 下方 → `"model": "opencode-go/qwen3.7-plus"`
         - `"artistry":` 下方 → `"model": "opencode-go/qwen3.7-plus"`
      4. 确认以下项仍为 `deepseek-v4-flash`：
         - sisyphus、hephaestus、librarian、explore、sisyphus-junior、atlas
         - visual-engineering、quick、unspecified-low、unspecified-high、writing
      5. 确认 multimodal-looker 仍为 `mimo-v2.5`
    Expected Result: 7 处 qwen3.7-plus 和其余项均正确
    Failure Indicators: 
      - 出现次数不等于 7
      - 被替换的项仍为 flash
      - 不应改动的项被改了
    Evidence: .omo/evidence/task-1-config-verified.log
  ```

  **Evidence to Capture**:
  - [ ] 最终配置文件内容保存到 `.omo/evidence/task-1-config-verified.log`

  **Commit**: NO（这不是项目代码，是用户级配置）
  - Pre-commit: N/A

---

## Success Criteria

### Verification Commands
```bash
$config = Get-Content "$env:USERPROFILE\.config\opencode\oh-my-openagent.json"
# 检查 qwen3.7-plus 出现 7 次
($config | Select-String -Pattern "qwen3.7-plus").Count -eq 7
# 检查 multimodal-looker 未被改动
$config -match "mimo-v2.5"
```

### Final Checklist
- [ ] Oracle 模型 → qwen3.7-plus
- [ ] Metis 模型 → qwen3.7-plus
- [ ] Momus 模型 → qwen3.7-plus
- [ ] Prometheus 模型 → qwen3.7-plus
- [ ] ultrabrain 分类 → qwen3.7-plus
- [ ] deep 分类 → qwen3.7-plus
- [ ] artistry 分类 → qwen3.7-plus
- [ ] 其余 11 项保持 flash 不变
- [ ] multimodal-looker 保持 mimo-v2.5

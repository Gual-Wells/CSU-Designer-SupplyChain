# CSU-Designer-SupplyChain

**中南大学本科毕业设计** · 面向企业供应链调度的多目标优化可视化实验平台

> A desktop visual workbench for multi-objective supply chain scheduling research, implementing a custom GA–PSO hybrid algorithm with Pareto archive management and interactive topology editing.

---

## 项目定位 / Overview

本项目是一个基于 **Qt/C++** 的桌面端交互式实验平台，面向供应链网络中的多目标订单调度问题。用户可在图形界面中自由构建供应链拓扑，配置节点属性与订单需求，运行自研混合优化算法求解 Pareto 前沿，并通过图表观察结果。

设计风格借鉴 **Boson NetSim Network Designer** 的交互范式——拓扑编辑与算法仿真阶段严格分离，无实时动画，强调场景复现与实验对照。

---

## 核心算法 / Algorithm

### 整体框架

算法以**多目标遗传算法（MOGA）为主体**，引入 PSO 的记忆与引导机制替代传统 GA 的选择压力，而非将两者并列使用。

```
主体结构：GA（种群演化、精英重组）
引导机制：PSO 概念（gbest / pbest / 惯性）
目标空间：二维 Pareto（总成本 × 总时效）+ 满意度代理惩罚项
```

### 路径探索核心：`order::exp()`

每个订单通过概率游走在供应链图上搜索从客户到仓库的可行路径，包含三项原创机制：

| 机制 | 描述 |
|------|------|
| **三维 reach 矩阵剪枝** | `reach[server][from][to]` 记录每个仓库分配下各有向边的可达性，失败即剪枝，独立于其他订单 |
| **brother\_order 动态合并** | 路径探索中实时检测同向订单，递归合并配送路径，共享成本与时效 |
| **false\_return 降级恢复** | 探索失败时从三级解集中采样历史解替代，保证种群活性 |

高速路与普通路在同一条边上共存，`randomHighway()` 按概率选择，建模司机选路的随机性（100 km/h vs 80 km/h）。

### PSO 引导的权重体系

三个权重 `a`、`b`、`c` 满足约束 `a + b + c = 1`，分别对应：

- **`a`** — 全局最优引导（订单种群精英解集内遗传）
- **`b`** — 个体历史最优引导（订单个体历史最优精英解集内遗传）
- **`c`** — 惯性（概率游走搜索）

每 20 代根据 Pareto 边界移动速率（`boundary_step`）与解集替换率（`replacement_rate`）自动调节三者比例。

### 三级 Pareto 解集

```
elite[]          全局 Pareto 前沿（主精英库）
elite_save[]     边界解保护库（被拥挤度淘汰但处于前沿边界的解）
p_best[][]       每订单独立 Pareto 历史最优集
```

拥挤度公式融合超时惩罚（满意度代理）与混合范数：

```
crowded = (2 - power/order_num)              // 超时率惩罚
        + α × √(p_c + t_c)                  // L2 分布稀疏度
        + (1-α) × (p_c_ + t_c_)             // L1 分布稀疏度
```

其中 `power`（超时订单数）作为用户满意度的代理指标，编码了"成本节省足以覆盖超时补偿"的商业语义。`α` 参数可在 GUI 中调节。

### GA / PP 算子

| 算子 | 描述 |
|------|------|
| `elite_GA()` | 从精英库逐个采样精英个体，逐订单随机重组，处理 child\_sisters 合并约束 |
| `elite_PP()` | 从 `p_best` 为每个订单采样历史最优，直接构造新种群 |
| `poweredge` 加权采样 | 余弦曲线权重，倾向采样 Pareto 前沿中部解，避免极端解主导 |

---

## 系统架构 / Architecture

项目分为五个清晰的层次：

```
Item 层       交互式图形编辑器（节点/边/场景/视图）
Data 层       领域数据模型（nodedata / edgedata / global 共享池）
搜索层        种群与订单路径探索（population_and_order.cpp）
解集层        Pareto 前沿管理与参数自适应（result.cpp）
可视化层      Pareto 散点图 / 解集选择（chartwidget / choosedialog）
```

搜索层与解集层形成**双向耦合**：`exp()` 调用 `catchorder()` 存储成功路径，失败时调用 `false_return()` 取历史解，构成"探索—记忆—恢复"闭环。

### 架构演化说明

`population.cpp`与`order.cpp`（全文注释保留）为重构前的历史快照。`order::exp()` 引入 `brother_order` 合并机制后，`order` 与 `population` 产生循环依赖，因此将 `order` 重构为 `population` 的嵌套类，两个文件合并为 `population_and_order.cpp`（1290 行）。注释文件为对照检查点，非冗余代码。

---

## 问题建模 / Problem Formulation

本项目不使用任何现成 benchmark，自定义约束体系如下：

| 要素 | 描述 |
|------|------|
| 节点类型 | 客户（Client）/ 仓库（Storage） |
| 商品 | 多商品类型，每类有库存与需求量 |
| 边属性 | 长度、普通路费用、高速路费用 |
| 时间窗 | 每个客户-商品对有期望送达时间（`hope_time`） |
| 库存隔离 | `live_goods[5]` 为五个种群各维护独立库存快照，避免并行探索产生库存竞争 |
| 订单合并 | 途经客户节点时可动态合并同向订单 |
| 优化目标 | 最小化总成本（price）与总时效（time） |

---

## 主要功能 / Features

- **交互式拓扑编辑器** — 拖拽放置节点，点击连边，双击弹出属性对话框，支持缩放与平移
- **参数控制面板** — 种群数、迭代步数、`α`、`save_point` 等参数 GUI 可调
- **Pareto 可视化** — Qt Charts 绘制 Pareto 散点图（X轴成本，Y轴时效）
- **解集选择对话框** — 从 Pareto 前沿中选取具体调度方案
- **场景持久化** — 完整 JSON 序列化/反序列化（节点、边、订单、算法参数），支持实验复现
- **策略消融实验** — 提供自构图场景，支持关闭单项机制与参数调整进行对照实验

---

## 技术栈 / Tech Stack

| 类别 | 技术 |
|------|------|
| GUI 框架 | Qt 5 / Qt 6（Widgets · GraphicsView · Charts） |
| 语言 | C++17 |
| 算法框架 | 自研 GA–PSO 混合多目标优化 |
| 持久化 | QJsonDocument（自定义 JSON schema） |
| 构建系统 | qmake（`.pro`） |
| 平台 | Windows / Linux / macOS |

---

## 快速开始 / Quick Start

**依赖：** Qt 5.12+ 或 Qt 6.x，含 Qt Charts 模块

```bash
# 克隆仓库
git clone https://github.com/Gual-Wells/CSU-Designer-SupplyChain.git
cd CSU-Designer-SupplyChain

# 方式一：Qt Creator
# 用 Qt Creator 打开 CSU_Designer_SupplyChain.pro，直接构建运行

# 方式二：命令行
qmake CSU_Designer_SupplyChain.pro
make -j$(nproc)
./CSU_Designer_SupplyChain
```

> **注意：** 构建时需确保 Qt Charts 模块已安装。Ubuntu 用户可通过 `sudo apt install libqt5charts5-dev` 补装。

---

## 使用流程 / Workflow

```
1. 在画布上放置仓库节点（Storage）与客户节点（Client）
2. 点击连线，设置边长度与运输费用
3. 双击节点配置库存、需求量与期望时效
4. 在参数面板设置种群数与迭代步数
5. 点击运行 → 观察 Pareto 散点图收敛过程
6. 从解集选择面板选取目标方案
7. 保存场景 JSON 以复现实验
```

---

## 已知局限 / Known Limitations

- 算法主循环在 UI 线程执行，运行期间界面暂停响应（因快速迭代开发代码耦合与质量问题，暂拟后续利用 `QThread` 更新多线程机制）
- 当前在 50–100 节点规模下运行良好；数百节点时 reach 矩阵内存压力显著上升，搜索效率下降；并且当前参数动态收敛策略此时不再易用
- 因快速迭代开发代码耦合与质量关系，尚未实现基线算法对照（NSGA-II / 随机搜索），消融实验基于自构图场景

---

## 目录结构 / Project Structure

```
├── main.cpp
├── mainwindow.cpp / .h          # 主控逻辑与算法主循环
├── population_and_order.cpp / .h # 种群结构 + 订单路径探索（核心）
├── result.cpp / .h              # Pareto 解集管理（核心）
├── fw.cpp / .h                  # JSON 场景序列化/反序列化
├── global.cpp / .h              # 全局共享状态池
├── nodedata.h / edgedata.h      # 领域数据模型
├── graphscene / graphview /     # Qt GraphicsView 图形层
│   nodeitem / edgeitem
├── chartwidget.cpp / .h         # Pareto 散点图可视化
├── choosedialog.cpp / .h        # 解集选择对话框
├── population.cpp               # 历史快照（全文注释，仅供参考）
├── order.cpp               # 历史快照（全文注释，仅供参考）
└── CSU_Designer_SupplyChain.pro
```

---

## License

MIT

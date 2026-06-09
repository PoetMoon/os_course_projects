# AB-MLFQ Scheduler in C

## 1. 项目简介

本项目实现了一个基于 C11 的进程调度模拟器，用于对比以下 5 种调度算法：

- `FCFS`
- `SJF`
- `RR`
- `Traditional MLFQ`
- `AB-MLFQ`

其中 `AB-MLFQ`（Adaptive Behavior-aware Multi-Level Feedback Queue）是重点算法。它在传统多级反馈队列基础上加入行为感知、饥饿补偿和自适应时间片调整机制，用于观察不同工作负载下的调度性能与公平性差异。

## 2. 算法说明

### FCFS

- 按到达时间先后执行
- 非抢占
- 运行到完成

### SJF

- 从已到达进程中选择运行时间最短者
- 非抢占
- 运行到完成

### RR

- 固定时间片 `quantum = 4`
- 用完时间片且未完成则重新进入队尾

### Traditional MLFQ

- 4 级队列：`Q0=2`、`Q1=4`、`Q2=8`、`Q3=16`
- 新进程进入 `Q0`
- 用完整个时间片则降级
- 等待时间超过 `aging_threshold=20` 可提升一级

### AB-MLFQ

- 在 MLFQ 基础上增加行为感知迁移
- 根据进程类型进行初始队列放置
- 延迟敏感任务可触发受限抢占
- 按等待阶段进行非线性 aging
- 周期性自适应时间片调整
- 输出饥饿率、公平性等指标

## 3. AB-MLFQ 创新点

### 行为画像

每个进程维护：

- `used_full_quantum_count`
- `early_yield_count`
- `current_waiting_streak`
- `max_waiting_streak`
- `demotion_count`
- `promotion_count`
- `aging_score`

### 行为感知队列迁移

- CPU 密集型进程更容易降级
- 提前结束时间片的进程被视为更偏交互型
- 后台批处理任务初始进入较低队列
- 延迟敏感任务保留在高优先级队列

### 延迟敏感任务抢占

当低优先级任务正在运行，而高优先级队列中出现延迟敏感任务时，AB-MLFQ 会触发受限抢占。该机制主要用于保护前台短任务响应，不对普通高优先级任务无条件抢占。

### 分段加速 aging

- 等待较短：`+0.2`
- 等待中等：`+0.6`
- 等待较长：`+1.2`

### 自适应时间片

每隔 20 个时间单位检查一次：

- 上下文切换频繁则增大低级队列时间片
- 平均响应过高则缩短高级队列时间片
- 饥饿率过高则降低 aging 触发阈值

## 4. 编译方式

```bash
cd ab_mlfq_scheduler_c
make
```

会生成可执行文件 `scheduler`。

## 5. 运行方式

支持两种运行方式。

### 方式一：位置参数

```bash
./scheduler all all 30 42
./scheduler ab_mlfq mixed 50 123
```

格式：

```text
./scheduler <algorithm> <scenario> <n> <seed>
```

### 方式二：命令行长参数

```bash
./scheduler --algorithm all --scenario mixed --n 30 --seed 42
```

### algorithm 可选值

- `all`
- `fcfs`
- `sjf`
- `rr`
- `mlfq`
- `ab_mlfq`

### scenario 可选值

- `all`
- `short_jobs`
- `long_jobs`
- `mixed`
- `interactive`
- `starvation_case`
- `behavior_sensitive`

## 6. 输出文件说明

运行后会生成：

- `results/summary.csv`
- `results/process_stats_{algorithm}_{scenario}.csv`
- `results/timeline_{algorithm}_{scenario}.csv`
- `results/ab_mlfq_debug_{scenario}.csv`（仅 AB-MLFQ）

绘图脚本会生成：

- `figures/avg_turnaround_time.png`
- `figures/avg_waiting_time.png`
- `figures/avg_response_time.png`
- `figures/context_switch_count.png`
- `figures/starvation_rate.png`
- `figures/fairness_index.png`

## 7. 指标说明

- `turnaround_time = finish_time - arrival_time`
- `waiting_time = turnaround_time - burst_time`
- `weighted_turnaround_time = turnaround_time / burst_time`
- `response_time = start_time - arrival_time`
- `waiting_time_variance`：等待时间方差
- `fairness_index = 1 / (1 + waiting_time_variance)`
- `starvation_count`：若 `max_waiting_streak >= 30` 则记为存在饥饿风险
- `cpu_utilization = busy_time / total_time`
- `throughput = completed_count / total_time`

## 8. 实验复现方式

完整复现命令：

```bash
make
./scheduler all all 30 42
python3 scripts/plot_results.py
```

## 9. 适合写进报告的分析思路

可以重点从以下几方面展开：

- 不同工作负载下各算法的平均周转时间与平均等待时间
- 交互型进程下响应时间表现
- `starvation_case` 场景中的饥饿率变化
- `behavior_sensitive` 场景中 AB-MLFQ 与传统 MLFQ 的响应时间差异
- AB-MLFQ 在前台响应和后台饥饿补偿之间的取舍
- 时间片自适应是否降低了上下文切换或响应时间
- aging 策略是否改善了低优先级长作业等待过长的问题

## 10. 简化说明

为了控制课程设计复杂度，本实验不模拟 I/O 阻塞，只关注就绪队列调度与 CPU 分配策略。报告中可明确说明本项目主要用于研究调度策略本身，而不是完整操作系统执行语义。

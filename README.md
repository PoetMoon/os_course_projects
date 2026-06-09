# 操作系统课程设计项目

本仓库是《操作系统》课程设计代码总目录，包含基础实验统一平台和拓展实验 AB-MLFQ 调度模拟器两部分。

## 项目内容

### 1. 基础实验统一平台

目录：`os_course_design/`

基础实验使用 C 语言实现一个菜单式命令行平台，包含：

- 处理机调度：FCFS、SJF、RR、优先级调度
- 内存管理：首次适应、最佳适应、FIFO 页面置换、LRU 页面置换
- 进程同步：生产者消费者、读者写者、哲学家进餐
- 模拟文件系统：目录、文件读写、删除、空闲块位图

### 2. AB-MLFQ 拓展实验

目录：`ab_mlfq_scheduler_c/`

拓展实验实现一个调度模拟器，对比：

- FCFS
- SJF
- RR
- Traditional MLFQ
- AB-MLFQ

其中 AB-MLFQ 是基于行为感知与饥饿补偿的自适应多级反馈队列调度算法。它重点面向 `behavior_sensitive` 场景，即少量后台 CPU 密集长任务和大量前台延迟敏感短任务并存的负载。

## 目录结构

```text
os_course_projects/
├── README.md
├── .gitignore
├── os_course_design/
│   ├── Makefile
│   ├── data/
│   ├── include/
│   └── src/
└── ab_mlfq_scheduler_c/
    ├── Makefile
    ├── include/
    ├── src/
    ├── scripts/
    ├── results/
    └── figures/
```


## 环境要求

基础环境：

- macOS / Linux
- `gcc`
- `make`
- `python3`

拓展实验画图脚本额外需要：

```bash
python3 -m pip install pandas matplotlib
```

## 快速开始

从仓库根目录开始：

```bash
git clone https://github.com/PoetMoon/os_course_projects.git
cd os_course_projects
```

### 运行基础实验

```bash
cd os_course_design
make
./os_course_design
```

程序主菜单：

```text
1. 处理机调度
2. 内存管理
3. 进程同步与并发控制
4. 模拟文件系统
0. 退出程序
```

调度模块支持手动输入和文件读取。标准样例位于：

```text
os_course_design/data/scheduler_cases/case1.txt
os_course_design/data/scheduler_cases/case2.txt
```

基础实验快速演示输入：

```text
1
2
5
2
0
0
```

含义：

- 进入处理机调度模块
- 读取标准样例 1
- 依次运行全部调度算法
- RR 时间片设置为 2
- 返回上级菜单
- 退出程序

清理编译产物：

```bash
make clean
```

### 运行拓展实验

```bash
cd ../ab_mlfq_scheduler_c
make
./scheduler all all 30 42
python3 scripts/plot_results.py
```

命令格式：

```text
./scheduler <algorithm> <scenario> <n> <seed>
```

也支持长参数：

```bash
./scheduler --algorithm all --scenario mixed --n 30 --seed 42
```

可选算法：

```text
all
fcfs
sjf
rr
mlfq
ab_mlfq
```

可选场景：

```text
all
short_jobs
long_jobs
mixed
interactive
starvation_case
behavior_sensitive
```

常用运行示例：

```bash
./scheduler all all 30 42
./scheduler ab_mlfq behavior_sensitive 30 42
./scheduler all behavior_sensitive 30 42
```

清理编译产物：

```bash
make clean
```

## 输出文件

拓展实验运行后会在 `ab_mlfq_scheduler_c/results/` 下生成：

- `summary.csv`
- `process_stats_{algorithm}_{scenario}.csv`
- `timeline_{algorithm}_{scenario}.csv`
- `ab_mlfq_debug_{scenario}.csv`

运行绘图脚本后会在 `ab_mlfq_scheduler_c/figures/` 下生成：

- `avg_turnaround_time.png`
- `avg_waiting_time.png`
- `avg_response_time.png`
- `context_switch_count.png`
- `starvation_rate.png`
- `fairness_index.png`

本仓库当前已经包含一组使用以下命令生成的实验结果：

```bash
./scheduler all all 30 42
python3 scripts/plot_results.py
```

## AB-MLFQ 实验重点

AB-MLFQ 的目标不是在所有负载下都取代经典算法，而是在行为差异明显的混合负载中优化前台响应和饥饿补偿。

核心机制：

- 行为画像：记录进程用满时间片、提前结束、等待 streak 等信息
- 行为感知初始分队：后台 CPU 密集任务进入较低队列，延迟敏感任务进入高优先级队列
- 延迟敏感抢占：高优先级延迟敏感任务到达时，可抢占低优先级任务
- 分段加速 aging：等待越久，aging 增长越快
- 自适应时间片：根据窗口内响应、切换和饥饿情况调整参数

重点对比场景：

```bash
cd ab_mlfq_scheduler_c
./scheduler all behavior_sensitive 30 42
```


## 推荐查看顺序

1. 阅读本文件了解总结构。
2. 进入 `os_course_design/` 运行基础实验。
3. 进入 `ab_mlfq_scheduler_c/` 运行拓展实验。
4. 查看 `ab_mlfq_scheduler_c/results/summary.csv`。
5. 查看 `ab_mlfq_scheduler_c/figures/` 中的图表。

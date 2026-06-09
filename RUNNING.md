# 运行说明

本文档用于统一说明两个课程设计项目的编译与运行方式。

## 一、环境要求

建议环境：

- macOS / Linux
- `gcc`
- `make`
- `python3`

拓展实验画图额外需要：

- `pandas`
- `matplotlib`

如未安装，可执行：

```bash
python3 -m pip install pandas matplotlib
```

## 二、基础实验运行方式

项目路径：

`os_course_design/`

### 1. 编译

```bash
cd os_course_design
make
```

### 2. 运行

```bash
./os_course_design
```

### 3. 功能说明

程序主菜单包含：

- `1` 处理机调度
- `2` 内存管理
- `3` 进程同步与并发控制
- `4` 模拟文件系统

### 4. 调度实验输入方式

调度模块支持两种形式：

- 手动输入
- 自动读取样例 / 自定义文件

进入调度模块后可选择：

- `1` 手动输入
- `2` 读取标准样例 1
- `3` 读取标准样例 2
- `4` 读取自定义文件

标准样例位于：

- `os_course_design/data/scheduler_cases/case1.txt`
- `os_course_design/data/scheduler_cases/case2.txt`

### 5. 基础实验快速演示

如果只想快速展示调度模块，可运行后按下面顺序输入：

```text
1
2
5
2
0
0
```

含义：

- 进入调度模块
- 读取标准样例 1
- 依次运行全部调度算法
- RR 时间片设为 2
- 返回
- 退出

## 三、拓展实验运行方式

项目路径：

`ab_mlfq_scheduler_c/`

### 1. 编译

```bash
cd ab_mlfq_scheduler_c
make
```

### 2. 批量运行全部实验

```bash
./scheduler all all 30 42
```

这条命令会运行：

- 全部 5 种调度算法
- 全部 5 类工作负载场景
- 进程数 `30`
- 随机种子 `42`

### 3. 运行单个算法示例

```bash
./scheduler ab_mlfq mixed 30 42
./scheduler fcfs short_jobs 20 123
./scheduler all behavior_sensitive 30 42
```

其中 `behavior_sensitive` 是 AB-MLFQ 的重点对比场景，模拟少量后台长任务和大量前台短交互任务并存的负载。

### 4. 使用长参数方式运行

```bash
./scheduler --algorithm all --scenario mixed --n 30 --seed 42
```

### 5. 结果文件

运行后会在 `results/` 下生成：

- `summary.csv`
- `process_stats_{algorithm}_{scenario}.csv`
- `timeline_{algorithm}_{scenario}.csv`
- `ab_mlfq_debug_{scenario}.csv`

### 6. 画图

```bash
python3 scripts/plot_results.py
```

生成图片目录：

`figures/`

包括：

- `avg_turnaround_time.png`
- `avg_waiting_time.png`
- `avg_response_time.png`
- `context_switch_count.png`
- `starvation_rate.png`
- `fairness_index.png`

## 四、推荐完整操作顺序

从总目录开始，建议这样操作：

```bash
cd "/Users/mac/Documents/New project/os_course_projects"
```

### 1. 运行基础实验

```bash
cd os_course_design
make
./os_course_design
cd ..
```

### 2. 运行拓展实验

```bash
cd ab_mlfq_scheduler_c
make
./scheduler all all 30 42
python3 scripts/plot_results.py
cd ..
```

## 五、GitHub 上传建议

建议把这个总目录作为一个仓库上传。

仓库根目录下已经包含：

- 顶层说明文件
- 两个子项目
- 适合忽略构建中间文件的 `.gitignore`

上传前如果你想让仓库更干净，可以重新编译一次，保留源码和实验结果即可。

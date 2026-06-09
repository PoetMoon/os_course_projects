# 操作系统课程设计项目总目录

这个总目录包含两部分课程设计内容：

- `os_course_design/`
  基础实验统一平台
- `ab_mlfq_scheduler_c/`
  拓展实验：`AB-MLFQ` 自适应多级反馈队列调度器

## 目录结构

```text
os_course_projects/
├── README.md
├── RUNNING.md
├── .gitignore
├── 操作系统课程设计报告.md
├── 基础实验_实验报告.md
├── AB-MLFQ_实验报告.md
├── os_course_design/
└── ab_mlfq_scheduler_c/
```

## 推荐上传方式

后续上传到 GitHub 时，建议直接把 `os_course_projects/` 作为仓库根目录。

例如：

```bash
cd "/Users/mac/Documents/New project/os_course_projects"
git init
git add .
git commit -m "Add OS course design projects"
```

## 两个项目说明

### 1. 基础实验

路径：

`os_course_design/`

包含：

- 处理机调度
- 内存管理
- 进程同步与并发控制
- 模拟文件系统

### 2. 拓展实验

路径：

`ab_mlfq_scheduler_c/`

包含：

- `FCFS`
- `SJF`
- `RR`
- `Traditional MLFQ`
- `AB-MLFQ`

并支持：

- 工作负载生成
- 指标统计
- CSV 输出
- Python 绘图

其中 `behavior_sensitive` 是 AB-MLFQ 的重点实验场景，用于验证该算法在“后台长任务 + 前台短交互任务”负载下的响应优化效果。

## 运行说明

统一运行说明见：

[RUNNING.md](/Users/mac/Documents/New%20project/os_course_projects/RUNNING.md)

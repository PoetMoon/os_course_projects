# OS Course Design

统一平台实现《操作系统》课程设计基础部分：

- 处理机调度：`FCFS`、`SJF`、`RR`、优先级调度
- 内存管理：动态分区 `FF`、`BF`；页面置换 `FIFO`、`LRU`
- 进程同步：生产者消费者、读者写者、哲学家进餐
- 文件系统：模拟目录与文件管理、空闲块位图、创建读写删除

## 构建

```bash
cd os_course_design
make
./os_course_design
```

## 说明

- 平台基于 `C + pthread + semaphore`。
- 文件系统为内存模拟版，适合作为课程设计展示与实验分析。

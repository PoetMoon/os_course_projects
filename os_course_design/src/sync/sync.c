#include "sync.h"

#include "common.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define PC_BUFFER_SIZE 5
#define PC_ROUNDS 6
#define RW_READER_COUNT 3
#define RW_WRITER_COUNT 2
#define DP_COUNT 5

static int pc_buffer[PC_BUFFER_SIZE];
static int pc_in = 0;
static int pc_out = 0;
static int pc_item = 1;
static pthread_mutex_t pc_mutex;
static sem_t pc_empty;
static sem_t pc_full;

static void *producer_thread(void *arg) {
    int id = *(int *)arg;
    for (int i = 0; i < PC_ROUNDS; ++i) {
        sem_wait(&pc_empty);
        pthread_mutex_lock(&pc_mutex);

        int value = pc_item++;
        pc_buffer[pc_in] = value;
        printf("生产者 %d 生产数据 %d，写入位置 %d\n", id, value, pc_in);
        pc_in = (pc_in + 1) % PC_BUFFER_SIZE;

        pthread_mutex_unlock(&pc_mutex);
        sem_post(&pc_full);
        usleep(50000);
    }
    return NULL;
}

static void *consumer_thread(void *arg) {
    int id = *(int *)arg;
    for (int i = 0; i < PC_ROUNDS; ++i) {
        sem_wait(&pc_full);
        pthread_mutex_lock(&pc_mutex);

        int value = pc_buffer[pc_out];
        printf("消费者 %d 消费数据 %d，读取位置 %d\n", id, value, pc_out);
        pc_out = (pc_out + 1) % PC_BUFFER_SIZE;

        pthread_mutex_unlock(&pc_mutex);
        sem_post(&pc_empty);
        usleep(70000);
    }
    return NULL;
}

static void run_producer_consumer(void) {
    pthread_t producers[2];
    pthread_t consumers[2];
    int producer_ids[2] = {1, 2};
    int consumer_ids[2] = {1, 2};

    print_divider("生产者消费者问题");
    pc_in = 0;
    pc_out = 0;
    pc_item = 1;
    pthread_mutex_init(&pc_mutex, NULL);
    sem_init(&pc_empty, 0, PC_BUFFER_SIZE);
    sem_init(&pc_full, 0, 0);

    for (int i = 0; i < 2; ++i) {
        pthread_create(&producers[i], NULL, producer_thread, &producer_ids[i]);
        pthread_create(&consumers[i], NULL, consumer_thread, &consumer_ids[i]);
    }
    for (int i = 0; i < 2; ++i) {
        pthread_join(producers[i], NULL);
        pthread_join(consumers[i], NULL);
    }

    sem_destroy(&pc_empty);
    sem_destroy(&pc_full);
    pthread_mutex_destroy(&pc_mutex);
    printf("生产者消费者实验结束。\n");
}

static int rw_data = 0;
static int rw_read_count = 0;
static pthread_mutex_t rw_count_mutex;
static sem_t rw_resource;
static sem_t rw_turnstile;

static void *reader_thread(void *arg) {
    int id = *(int *)arg;
    for (int i = 0; i < 3; ++i) {
        sem_wait(&rw_turnstile);
        sem_post(&rw_turnstile);

        pthread_mutex_lock(&rw_count_mutex);
        rw_read_count++;
        if (rw_read_count == 1) {
            sem_wait(&rw_resource);
        }
        pthread_mutex_unlock(&rw_count_mutex);

        printf("读者 %d 正在读取数据，当前值 = %d\n", id, rw_data);
        usleep(40000);

        pthread_mutex_lock(&rw_count_mutex);
        rw_read_count--;
        if (rw_read_count == 0) {
            sem_post(&rw_resource);
        }
        pthread_mutex_unlock(&rw_count_mutex);
        usleep(50000);
    }
    return NULL;
}

static void *writer_thread(void *arg) {
    int id = *(int *)arg;
    for (int i = 0; i < 3; ++i) {
        sem_wait(&rw_turnstile);
        sem_wait(&rw_resource);

        rw_data += id;
        printf("写者 %d 正在写入数据，新值 = %d\n", id, rw_data);
        usleep(60000);

        sem_post(&rw_resource);
        sem_post(&rw_turnstile);
        usleep(80000);
    }
    return NULL;
}

static void run_reader_writer(void) {
    pthread_t readers[RW_READER_COUNT];
    pthread_t writers[RW_WRITER_COUNT];
    int reader_ids[RW_READER_COUNT] = {1, 2, 3};
    int writer_ids[RW_WRITER_COUNT] = {1, 2};

    print_divider("读者写者问题");
    rw_data = 0;
    rw_read_count = 0;
    pthread_mutex_init(&rw_count_mutex, NULL);
    sem_init(&rw_resource, 0, 1);
    sem_init(&rw_turnstile, 0, 1);

    for (int i = 0; i < RW_READER_COUNT; ++i) {
        pthread_create(&readers[i], NULL, reader_thread, &reader_ids[i]);
    }
    for (int i = 0; i < RW_WRITER_COUNT; ++i) {
        pthread_create(&writers[i], NULL, writer_thread, &writer_ids[i]);
    }
    for (int i = 0; i < RW_READER_COUNT; ++i) {
        pthread_join(readers[i], NULL);
    }
    for (int i = 0; i < RW_WRITER_COUNT; ++i) {
        pthread_join(writers[i], NULL);
    }

    sem_destroy(&rw_resource);
    sem_destroy(&rw_turnstile);
    pthread_mutex_destroy(&rw_count_mutex);
    printf("读者写者实验结束。\n");
}

static pthread_mutex_t forks[DP_COUNT];
static sem_t dining_room;

static void *philosopher_thread(void *arg) {
    int id = *(int *)arg;
    int left = id;
    int right = (id + 1) % DP_COUNT;

    for (int i = 0; i < 3; ++i) {
        printf("哲学家 %d 正在思考\n", id + 1);
        usleep(50000);

        sem_wait(&dining_room);
        pthread_mutex_lock(&forks[left]);
        pthread_mutex_lock(&forks[right]);

        printf("哲学家 %d 正在进餐\n", id + 1);
        usleep(50000);

        pthread_mutex_unlock(&forks[right]);
        pthread_mutex_unlock(&forks[left]);
        sem_post(&dining_room);
    }
    return NULL;
}

static void run_dining_philosophers(void) {
    pthread_t philosophers[DP_COUNT];
    int ids[DP_COUNT] = {0, 1, 2, 3, 4};

    print_divider("哲学家进餐问题");
    sem_init(&dining_room, 0, DP_COUNT - 1);
    for (int i = 0; i < DP_COUNT; ++i) {
        pthread_mutex_init(&forks[i], NULL);
    }
    for (int i = 0; i < DP_COUNT; ++i) {
        pthread_create(&philosophers[i], NULL, philosopher_thread, &ids[i]);
    }
    for (int i = 0; i < DP_COUNT; ++i) {
        pthread_join(philosophers[i], NULL);
    }
    for (int i = 0; i < DP_COUNT; ++i) {
        pthread_mutex_destroy(&forks[i]);
    }
    sem_destroy(&dining_room);
    printf("哲学家进餐实验结束。\n");
}

void sync_menu(void) {
    int choice = -1;

    while (choice != 0) {
        print_divider("进程同步与并发控制");
        printf("1. 生产者消费者\n");
        printf("2. 读者写者\n");
        printf("3. 哲学家进餐\n");
        printf("4. 依次运行全部实验\n");
        printf("0. 返回上一级\n");
        choice = read_int("请选择实验: ");

        switch (choice) {
            case 1:
                run_producer_consumer();
                break;
            case 2:
                run_reader_writer();
                break;
            case 3:
                run_dining_philosophers();
                break;
            case 4:
                run_producer_consumer();
                run_reader_writer();
                run_dining_philosophers();
                break;
            case 0:
                break;
            default:
                printf("无效选项。\n");
                break;
        }
    }
}

/*
 gcc test.c -o test
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <time.h>

#define FTOK_PATH "/tmp"
#define EMAIL_PROJ_ID 'E'
#define TIMER_PROJ_ID_BASE 'A'
#define MSG_SIZE 256
#define NUM_TIMERS 1 // 可擴充多個 Timer Thread

struct msgbuf {
    long mtype;
    char mtext[MSG_SIZE];
};

typedef struct {
    int msqid; // Email queue
} email_info_t;

typedef struct {
    int timer_id;
    int interval_sec;
    int msqid;       // Timer 自己 queue
    int email_msqid; // Email queue
} timer_info_t;

// 發送訊息到 queue
void send_message(int msqid, const char* text) {
    struct msgbuf msg;
    msg.mtype = 1;
    strncpy(msg.mtext, text, MSG_SIZE);
    if (msgsnd(msqid, &msg, strlen(msg.mtext)+1, 0) == -1) {
        perror("msgsnd failed");
    }
}

// Timer Thread: 每隔 interval 秒送訊息到 Email Thread，然後接收回覆
void* timer_thread_func(void* arg) {
    timer_info_t* info = (timer_info_t*)arg;
    int count = 0;
    struct msgbuf reply;

    while (1) {
        sleep(info->interval_sec);
        char text[MSG_SIZE];
        snprintf(text, sizeof(text), "Timer %d msg %d at %ld",
                 info->timer_id, ++count, time(NULL));
        printf("[Timer %d] Sending: %s\n", info->timer_id, text);

        // 發送給 Email Thread
        send_message(info->email_msqid, text);

        // 接收 Email Thread 回覆
        if (msgrcv(info->msqid, &reply, sizeof(reply.mtext), 0, 0) != -1) {
            printf("[Timer %d] Received reply: %s\n", info->timer_id, reply.mtext);
        }
    }
    return NULL;
}

// Email Thread: 接收 Timer Thread 訊息後回覆
void* email_thread_func(void* arg) {
    email_info_t* info = (email_info_t*)arg;
    struct msgbuf msg;

    while (1) {
        if (msgrcv(info->msqid, &msg, sizeof(msg.mtext), 0, 0) != -1) {
            printf("[Email Thread] Received: %s\n", msg.mtext);

            // 從訊息內容解析 timer_id（假設格式 Timer X msg ...）
            int timer_id;
            if (sscanf(msg.mtext, "Timer %d", &timer_id) == 1) {
                key_t timer_key = ftok(FTOK_PATH, TIMER_PROJ_ID_BASE + timer_id - 1);
                int timer_msqid = msgget(timer_key, 0666);
                if (timer_msqid != -1) {
                    char reply_text[MSG_SIZE];
                    snprintf(reply_text, sizeof(reply_text), "Reply to Timer %d at %ld", timer_id, time(NULL));
                    send_message(timer_msqid, reply_text);
                }
            }
        }
    }
    return NULL;
}

int main() {
    pthread_t email_thread, timer_threads[NUM_TIMERS];
    email_info_t email_info;
    timer_info_t timer_infos[NUM_TIMERS];

    // 建立 Email queue
    key_t email_key = ftok(FTOK_PATH, EMAIL_PROJ_ID);
    email_info.msqid = msgget(email_key, IPC_CREAT | 0666);
    if (email_info.msqid == -1) {
        perror("msgget failed for email");
        exit(1);
    }

    pthread_create(&email_thread, NULL, email_thread_func, &email_info);

    // 建立 Timer thread(s)
    for (int i = 0; i < NUM_TIMERS; i++) {
        timer_infos[i].timer_id = i + 1;
        timer_infos[i].interval_sec = 2;

        // Timer 自己 queue
        key_t timer_key = ftok(FTOK_PATH, TIMER_PROJ_ID_BASE + i);
        timer_infos[i].msqid = msgget(timer_key, IPC_CREAT | 0666);
        if (timer_infos[i].msqid == -1) {
            perror("msgget failed for timer");
            exit(1);
        }

        timer_infos[i].email_msqid = email_info.msqid;

        pthread_create(&timer_threads[i], NULL, timer_thread_func, &timer_infos[i]);
    }

    // 等待 threads
    pthread_join(email_thread, NULL);
    for (int i = 0; i < NUM_TIMERS; i++)
        pthread_join(timer_threads[i], NULL);

    // 刪除 message queue
    msgctl(email_info.msqid, IPC_RMID, NULL);
    for (int i = 0; i < NUM_TIMERS; i++)
        msgctl(timer_infos[i].msqid, IPC_RMID, NULL);

    return 0;
}


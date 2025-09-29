#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#define MQ_NAME "/test_mq"
#define MAX_MSG 10
#define MSG_SIZE 128

// 先宣告 mqd_t 為全域，方便 send / notify 使用
mqd_t mqd;

// 當訊息到達時的回呼
void notify_handler(union sigval sv) {
    char buf[MSG_SIZE];
    ssize_t n;

    // 讀取訊息
    n = mq_receive(mqd, buf, MSG_SIZE, NULL);
    if (n >= 0) {
        buf[n] = '\0';
        printf("[Receiver] Got message: %s\n", buf);
    } else {
        perror("mq_receive");
    }

    // 重新註冊通知
    struct sigevent sev;
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = notify_handler;
    sev.sigev_notify_attributes = NULL;
    sev.sigev_value.sival_ptr = NULL;
    if (mq_notify(mqd, &sev) == -1) {
        perror("mq_notify");
    }
}

// 發送訊息的 thread
void *sender_thread(void *arg) {
    int count = 0;
    char msg[MSG_SIZE];

    while (1) {
        snprintf(msg, MSG_SIZE, "Message %d", count++);
        if (mq_send(mqd, msg, strlen(msg), 0) == -1) {
            perror("mq_send");
        } else {
            printf("[Sender] Sent: %s\n", msg);
        }
        sleep(2); // 每 2 秒發送一次
    }
    return NULL;
}

int main() {
    struct mq_attr attr;
    struct sigevent sev;
    pthread_t tid;

    // 設定訊息佇列屬性
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MSG;
    attr.mq_msgsize = MSG_SIZE;
    attr.mq_curmsgs = 0;

    // 開啟或建立訊息佇列
    mqd = mq_open(MQ_NAME, O_CREAT | O_RDWR, 0644, &attr);
    if (mqd == (mqd_t)-1) {
        perror("mq_open");
        exit(1);
    }

    // 設定 mq_notify
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = notify_handler;
    sev.sigev_notify_attributes = NULL;
    sev.sigev_value.sival_ptr = NULL;

    if (mq_notify(mqd, &sev) == -1) {
        perror("mq_notify");
        exit(1);
    }

    // 建立 sender thread
    if (pthread_create(&tid, NULL, sender_thread, NULL) != 0) {
        perror("pthread_create");
        exit(1);
    }

    printf("Sender and Receiver running...\n");

    // 主程式休眠等待 thread 執行
    while (1) {
        pause(); // 等待通知或中斷
    }

    mq_close(mqd);
    mq_unlink(MQ_NAME);
    return 0;
}


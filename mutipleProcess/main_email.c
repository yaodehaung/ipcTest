#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <time.h>
#include "ipc_common.h"

volatile int running = 1;

int get_offset(long type) {
    return (type - 1) * SEGMENT_SIZE;
}

void send_message(int msgid, long self_type, long target, char *shm, const char *data, int len) {
    int offset = get_offset(self_type);
    if (len > 0)
        memcpy(shm + offset, data, len);

    message_t msg;
    msg.mtype = target;
    msg.size = len;
    msg.offset = offset;
    snprintf(msg.mtext, MSG_SIZE, "From EMAIL");
    msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0);

    printf("[EMAIL] Sent %d bytes to TIMER | content=", len);
    for (int i = 0; i < len; i++) putchar(shm[offset + i]);
    printf("\n");
}

void recv_message(int msgid, char *shm) {
    message_t msg;
    while (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), EMAIL_PROCESS, IPC_NOWAIT) > 0) {
        char buffer[msg.size + 1];
        memcpy(buffer, shm + msg.offset, msg.size);
        buffer[msg.size] = '\0';

        printf("[EMAIL] Received %d bytes from offset=%d | control=%s | content=",
               msg.size, msg.offset, msg.mtext);
        for (int i = 0; i < msg.size; i++) putchar(buffer[i]);
        printf("\n");

        if (strncmp(buffer, "exit", 4) == 0)
            running = 0;
    }
}

int main() {
    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    int shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    char *shm = (char *)shmat(shmid, NULL, 0);

    if (msgid == -1 || shmid == -1 || shm == (char *)-1) {
        perror("IPC setup failed");
        exit(1);
    }

    srand(time(NULL));

    while (running) {
        // 發送訊息給 timer process
        char data[20];
        snprintf(data, sizeof(data), "Hello TIMER %ld", time(NULL));
        send_message(msgid, EMAIL_PROCESS, TIMER_PROCESS, shm, data, strlen(data));

        recv_message(msgid, shm);

        sleep(2); // 每 2 秒發送一次
    }

    shmdt(shm);
    printf("[EMAIL] Exiting.\n");
    return 0;
}


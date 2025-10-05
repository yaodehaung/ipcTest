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
    snprintf(msg.mtext, MSG_SIZE, "From TIMER");
    msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0);

    printf("[TIMER] Sent %d bytes to EMAIL | content=", len);
    for (int i = 0; i < len; i++) putchar(shm[offset + i]);
    printf("\n");
}

void recv_message(int msgid, char *shm) {
    message_t msg;
    while (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), TIMER_PROCESS, IPC_NOWAIT) > 0) {
        char buffer[msg.size + 1];
        memcpy(buffer, shm + msg.offset, msg.size);
        buffer[msg.size] = '\0';

        printf("[TIMER] Received %d bytes from offset=%d | control=%s | content=",
               msg.size, msg.offset, msg.mtext);
        for (int i = 0; i < msg.size; i++) putchar(buffer[i]);
        printf("\n");

        if (strncmp(buffer, "exit", 4) == 0)
            running = 0;
        else {
            // 回覆 EMAIL process
            char reply[20];
            snprintf(reply, sizeof(reply), "ACK %ld", time(NULL));
            send_message(msgid, TIMER_PROCESS, EMAIL_PROCESS, shm, reply, strlen(reply));
        }
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

    while (running) {
        recv_message(msgid, shm);
        usleep(100000); // 100ms
    }

    shmdt(shm);
    printf("[TIMER] Exiting.\n");
    return 0;
}


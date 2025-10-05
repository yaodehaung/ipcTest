#include "ipc_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

int main() {
    int msgid = msgget(MSG_KEY, 0666);
    int shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
    char *shm = (char *)shmat(shmid, NULL, 0);

    if (msgid == -1 || shmid == -1 || shm == (char *)-1) {
        perror("IPC setup failed");
        exit(1);
    }

    char exit_msg[] = "exit";

    // 對每個 process 寫入它自己的 shared memory offset
    for (long i = EMAIL_PROCESS; i <= TIMER_PROCESS; i++) {
        int offset = (i - 1) * SEGMENT_SIZE;  // ✅ 每個 process 對應的 offset
        strcpy(shm + offset, exit_msg);

        message_t msg;
        msg.mtype = i;
        msg.size = strlen(exit_msg);
        msg.offset = offset;   // ✅ 設定對應 offset
        strcpy(msg.mtext, "exit");
        msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0);
    }

    shmdt(shm);
    printf("[Main] Broadcast exit complete.\n");
    return 0;
}


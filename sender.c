#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <string.h>

struct msgbuf {
    long mtype;
};

int main() {
    key_t shm_key = 5678;
    key_t msg_key = 1234;

    // 1. ���� shared memory
    int shmid = shmget(shm_key, 1024, IPC_CREAT | 0666);
    char *data = shmat(shmid, NULL, 0);
    strcpy(data, "Hello via shared memory!");
    
    // 2. ���� message queue
    int msqid = msgget(msg_key, IPC_CREAT | 0666);
    struct msgbuf msg = {1};
    
    // 3. �l��֪ͨ
    msgsnd(msqid, &msg, 0, 0);

    printf("Sender: data written and notification sent.\n");
    shmdt(data);
    return 0;
}

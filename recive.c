#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>

struct msgbuf {
    long mtype;
};

int main() {
    key_t shm_key = 5678;
    key_t msg_key = 1234;

    // 1.  message queue
    int msqid = msgget(msg_key, 0666);
    struct msgbuf msg;
    
    // 2.  sender 
    msgrcv(msqid, &msg, 0, 0, 0);

    // 3.  shared memory
    int shmid = shmget(shm_key, 1024, 0666);
    char *data = shmat(shmid, NULL, 0);
    printf("Receiver: read data: %s\n", data);

    shmdt(data);
    return 0;
}

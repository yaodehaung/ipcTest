#ifndef IPC_COMMON_H
#define IPC_COMMON_H

#define PROCESS_COUNT 2       // email + timer
#define SEGMENT_SIZE 512
#define SHM_SIZE (PROCESS_COUNT * SEGMENT_SIZE)
#define MSG_SIZE 128
#define MSG_KEY 1234
#define SHM_KEY 5678

// 定義特殊 process
#define EMAIL_PROCESS 1
#define TIMER_PROCESS 2

typedef struct {
    long mtype;          // 接收 process
    int  size;           // shared memory 寫入長度
    int  offset;         // shared memory 偏移
    char mtext[MSG_SIZE];// 控制訊息
} message_t;

#endif


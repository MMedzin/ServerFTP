#include <pthread.h>


#ifndef SK_PROJEKT_THREAD_DATA_T_H
#define SK_PROJEKT_THREAD_DATA_T_H
struct thread_data_t
{
    int fd;
    pthread_mutex_t mutex;
    int doExit;
    int fileTransferConn;
    char* fileTransferAddress;
    int fileTransferPort;
    int transferMode;
    char* username;
    char wDir[1035];
    char* filename;
};
#endif //SK_PROJEKT_THREAD_DATA_T_H

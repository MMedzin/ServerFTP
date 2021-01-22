#include <pthread.h>


#ifndef SK_PROJEKT_THREAD_DATA_T_H
#define SK_PROJEKT_THREAD_DATA_T_H
struct thread_data_t
{
    int fd;
    pthread_mutex_t mutex;
    int do_exit;
    int fd_file_transfer;
    char* file_transfer_address;
    int file_transfer_port;
    int transfer_mode;
    char* username;
    char working_directory[1035];
    char* filename;
};
#endif //SK_PROJEKT_THREAD_DATA_T_H

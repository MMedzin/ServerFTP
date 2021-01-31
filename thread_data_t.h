#include <pthread.h>

#include "hashmap_threads.h"

#ifndef SK_PROJEKT_THREAD_DATA_T_H
#define SK_PROJEKT_THREAD_DATA_T_H

//struktura zawierająca dane, które zostaną przekazane do wątku
struct thread_data_t {
    int fd; //deskryptor pliku socketa
    struct table *mutex_table; //tablica mutexów
    int do_exit; //
    int fd_file_transfer;
    char *file_transfer_address;
    int file_transfer_port;
    int transfer_mode;
    char *username;
    char working_directory[1035];
};
#endif //SK_PROJEKT_THREAD_DATA_T_H

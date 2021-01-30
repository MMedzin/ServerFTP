#include "thread_data_t.h"
#ifndef SK_PROJEKT_COMMANDS_H
#define SK_PROJEKT_COMMANDS_H

#define USER "test"
#define PASS "testhaslo"

int user_cmd(void *thr_data, char* args);
int stor_cmd(void *thr_data, char *args);
int rmd_cmd(void *thr_data, char *args);
int dele_cmd(void *thr_data, char* args);
int retr_cmd(void *thr_data, char *args);
int list_cmd(void *thr_data);
int cdup_cmd(void *thr_data);
int cwd_cmd(void *thr_data, char* args);
int mkd_cmd(void *thr_data, char* args);
void * sendList(void *t_data);
int transform_port_number(char *p1, char *p2);

#endif //SK_PROJEKT_COMMANDS_H

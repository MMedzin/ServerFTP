//
// Created by marysia on 20.01.2021.
//

#include "thread_data_t.h"
#ifndef SK_PROJEKT_COMMANDS_H
#define SK_PROJEKT_COMMANDS_H

#define USER "test1"
#define PASS "testhaslo"

int user_cmd(void *thr_data, char* ptr);
int stor_cmd(void *thr_data, char* ptr);
int rmd_cmd(void *thr_data, char* ptr);
int dele_cmd(void *thr_data, char* ptr);
int list_cmd(void *thr_data);
int cdup_cmd(void *thr_data);
int mkd_cmd(void *thr_data, char* ptr);
void * sendList(void *t_data);
int transformPortNumber(char p1[], char p2[]);

#endif //SK_PROJEKT_COMMANDS_H

//
// Created by marysia on 24.01.2021.
//

#ifndef SK_PROJEKT_HASHMAP_THREADS_H
#define SK_PROJEKT_HASHMAP_THREADS_H
struct table *createTable(int size);
int hashCode(struct table *t,int key);
int convertStrToInt(char * name);
void insert(struct table *t, char* val_str);
pthread_mutex_t lookup(struct table *t,char * val_str);
void clearTable(struct table *t);
int delete(struct table *t, char * val_str);
#endif //SK_PROJEKT_HASHMAP_THREADS_H

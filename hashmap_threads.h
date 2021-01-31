//
// Created by marysia on 24.01.2021.
//

#ifndef SK_PROJEKT_HASHMAP_THREADS_H
#define SK_PROJEKT_HASHMAP_THREADS_H

struct table *createTable(int size); //tworzenie tabeli hashowej
int hashCode(struct table *t, int key); //obliczanie hash kodu
int convertStrToInt(char *name); //zamiana stringa na inta (suma po kodach ASCII znaków)
void insert(struct table *t, char *val_str); //wstawianie do tabeli
pthread_mutex_t lookup(struct table *t, char *val_str); //wyszukiwanie w tabeli
void clearTable(struct table *t); //czyszczenie tablicy
int delete(struct table *t, char *val_str); //usuwanie z tablicy

#endif //SK_PROJEKT_HASHMAP_THREADS_H

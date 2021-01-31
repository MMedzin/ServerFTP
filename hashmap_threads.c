#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "hashmap_threads.h"

//Hashmapa odpowiedzialna za składowanie mutexów dla każdego z plików

//Węzeł w hashmapie
struct node {
    int key;
    pthread_mutex_t val;
    struct node *next;
};

//Tablica węzłów
struct table {
    int size;
    struct node **list;
};

//Funkcja inicjalizująca tablicę o zadanym rozmiarze
struct table *createTable(int size) {
    struct table *t = (struct table *) malloc(sizeof(struct table));
    t->size = size;
    t->list = (struct node **) malloc(sizeof(struct node *) * size);
    int i;
    for (i = 0; i < size; i++)
        t->list[i] = calloc(1, sizeof(struct node));
    return t;
}

//Funkcja haszująca
int hashCode(struct table *t, int key) {
    if (key < 0)
        return -(key % t->size);
    return key % t->size;
}

//Zamiana stringa w integera, sumując po kodach ASCII
int convertStrToInt(char *name) {
    int size = (int) strlen(name);
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += (int) name[i];
    }

    return sum;
}

//Procedura wstawiająca nowy węzeł
void insert(struct table *t, char *val_str) {
    int key = convertStrToInt(val_str);
    int pos = hashCode(t, key);
    struct node *list = t->list[pos];
    struct node *newNode = (struct node *) malloc(sizeof(struct node));
    struct node *temp = list;
    while (temp) {
        if (temp->key == key) {
            pthread_mutex_init(&(temp->val), NULL);
            return;
        }
        temp = temp->next;
    }
    newNode->key = key;
    pthread_mutex_init(&(newNode->val), NULL);
    newNode->next = list;
    free(t->list[pos]);
    t->list[pos] = newNode;
}

int delete(struct table *t, char *val_str) {
    int key = convertStrToInt(val_str);
    int pos = hashCode(t, key);
    struct node *list = t->list[pos];
    struct node *temp = list;
    while (temp) {
        if (temp->key == key) {
            free(t->list[pos]);
            t->list[pos] = calloc(1, sizeof(struct node));
            return 0;
        }
        temp = temp->next;
    }
    return -1;
}


//Funkcja szukająca w tablicy mutexa dla danej nazwy pliku. Jeżeli nie odnaleziono, to zostaje utworzony i wstawiony do tablicy
pthread_mutex_t lookup(struct table *t, char *val_str) {
    int key = convertStrToInt(val_str);
    int pos = hashCode(t, key);
    struct node *list = t->list[pos];
    struct node *temp = list;
    while (temp) {
        if (temp->key == key) {
            return temp->val;
        }
        temp = temp->next;
    }

    insert(t, val_str);
    return lookup(t, val_str);
}

//Funkcja zwalniająca pamięć
void clearTable(struct table *t) {
    int size = t->size;
    int i;
    for (i = 0; i < size; i++) {
        if (t->list[i] != NULL) {
            free(t->list[i]);
        }
    }

    free(t->list);
    free(t);
}

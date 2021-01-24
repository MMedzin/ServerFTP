#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "hashmap_threads.h"

struct node{
    int key;
    pthread_mutex_t val;
    struct node *next;
};
struct table{
    int size;
    struct node **list;
};
struct table *createTable(int size){
    struct table *t = (struct table*)malloc(sizeof(struct table));
    t->size = size;
    t->list = (struct node**)malloc(sizeof(struct node*)*size);
    int i;
    for(i=0;i<size;i++)
        t->list[i] = NULL;
    return t;
}
int hashCode(struct table *t,int key){
    if(key<0)
        return -(key%t->size);
    return key%t->size;
}

int convertStrToInt(char * name){
    int size= strlen(name);
    int sum=0;
    for(int i=0;i<size;i++){
        sum+= (int) name[i];
    }

    return sum;
}

void insert(struct table *t, char* val_str){
    int key = convertStrToInt(val_str);
    int pos = hashCode(t,key);
    struct node *list = t->list[pos];
    struct node *newNode = (struct node*)malloc(sizeof(struct node));
    struct node *temp = list;
    while(temp){
        if(temp->key==key){
            pthread_mutex_init(&(temp->val), NULL);
            return;
        }
        temp = temp->next;
    }
    newNode->key = key;
    pthread_mutex_init(&(newNode->val), NULL);
    newNode->next = list;
    t->list[pos] = newNode;
}
pthread_mutex_t lookup(struct table *t,char * val_str){
    int key = convertStrToInt(val_str);
    int pos = hashCode(t,key);
    struct node *list = t->list[pos];
    struct node *temp = list;
    while(temp){
        if(temp->key==key){
            return temp->val;
        }
        temp = temp->next;
    }
    exit(-1);
}

#include "commands.h"
#include "thread_data_t.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ftw.h>
#define BUF_SIZE 1000
// Created by marysia on 20.01.2021.
//


int user_cmd(void *thr_data, char* ptr){
    struct thread_data_t *th_data = (struct thread_data_t*)thr_data;
    char *dir_name = ptr;
    mkdir(dir_name, 0777);
    (*th_data).username = USER;
    strcpy((*th_data).wDir, "/");
    strcat((*th_data).wDir, dir_name);
    printf("USER correct, need pass: 331 ->\n");
    return 0;
}

int stor_cmd(void *thr_data, char* ptr){

//            char** nameParts = malloc(50*sizeof(char*));
    struct thread_data_t *th_data = (struct thread_data_t*)thr_data;
    char * filename;
    filename = calloc(strlen((*th_data).wDir) + strlen(ptr) + 2, sizeof(char));
    strcpy(filename, ".");
    strcat(filename, (*th_data).wDir);
    strcat(filename, "/");
    strcat(filename, ptr);
    printf("Nazwa pliku: %s\n", filename);
    write((th_data)->fd, "150 Opening ASCII mode data connection for file list.\r\n",
          strlen("150 Opening ASCII mode data connection for file list.\r\n"));

    FILE *fp;
    char buffer[BUF_SIZE];
    char bigBuffer[1000*BUF_SIZE];

    fp = fopen(filename, "w");
    // TODO dodaj tu coś jak się plik źle otworzy
    int n = read((*th_data).fileTransferConn, buffer, BUF_SIZE);
    buffer[n] = '\0';
    strcpy(bigBuffer, buffer);
    while(n>0){
        bzero(buffer, BUF_SIZE);
        n = read((*th_data).fileTransferConn, buffer, BUF_SIZE);
        buffer[n]='\0';
        strcat(bigBuffer, buffer);
    }
    // TODO jak coś się źle odczyta
    strcat(bigBuffer, "\0");

    fprintf(fp, "%s", bigBuffer);
    printf("%s\n", bigBuffer);
    bzero(buffer, BUF_SIZE);
    bzero(bigBuffer, 1000*BUF_SIZE);
    fclose(fp);

//    close((*th_data).fd);
    return 0;
}

int rmd_cmd(void*thr_data, char *ptr) {
    struct thread_data_t *th_data = (struct thread_data_t *) thr_data;
    if (ptr != NULL) {
        char *pathToDir = calloc(strlen((*th_data).wDir) + strlen(ptr) + 2, sizeof(char));
        strcat(pathToDir, ".");
        strcat(pathToDir, (*th_data).wDir);
        strcat(pathToDir, "/");
        strcat(pathToDir, ptr);
        printf("Removing %s...\n", pathToDir);
        rmdir(pathToDir);
        //TODO kontrola usuwania
        free(pathToDir);
        printf("Directory removed\n");
        return 0;
    }
    return -1;
}

// funkcja tworząca wątek odpowiedzialny za przesłanie listy folderu
int list_cmd(void *thr_data){

    struct thread_data_t *th_data = (struct thread_data_t*)thr_data;

    pthread_t thread;
    struct thread_data_t *t_data = malloc(sizeof(struct thread_data_t));
    t_data->doExit = 0;
    t_data->fd = (*th_data).fd;
    t_data->username = (*th_data).username;
    strcpy(t_data->wDir, (*th_data).wDir);
    t_data->fileTransferConn = (*th_data).fileTransferConn;
    int create_result = 0;
    create_result = pthread_create(&thread, NULL, sendList, (void *)t_data);
    if (create_result){
        printf("Błąd przy próbie utworzenia wątku przesyłania listy, kod błędu: %d\n", create_result);
        exit(-1);
    }

    return 0;
}

void *sendList(void *t_data) {

    pthread_detach(pthread_self());
    struct thread_data_t *th_data = (struct thread_data_t*)t_data;

    FILE *fp;
    char path[1035];
    char command[1047] = "/bin/ls -l .";
    strcat(command, (*th_data).wDir);
    fp = popen(command, "r");
    if(fp == NULL) {
        printf("failed to read directory\n");
    }
    else{
        unsigned long i = 0;
        while (fgets(path, sizeof(path), fp) != NULL){
//            strcat(path, "\r\n");
            i = strlen(path);
            path[i-1] = '\r';
            path[i] = '\n';
            path[i+1] = '\0';
            write((*th_data).fileTransferConn, path, strlen(path));
        }
        pclose(fp);
    }

    close((*th_data).fileTransferConn);
    write((*th_data).fd, "226 Transfer complete.\r\n", strlen("226 Transfer complete.\r\n"));
    printf("226 Transfer complete.\r\n");
    free(th_data);
    pthread_exit(NULL);
}

int cdup_cmd(void *thr_data){
    struct thread_data_t *th_data = (struct thread_data_t*)thr_data;
    char * dir = (*th_data).wDir;
    long d = strlen(dir);
    long i;
    char delim = '/';

    for(i=d-1;i>=0;i--){
        if(dir[i]==delim && i != d-1){
            break;
        }
    }
    long toCutLen = d-i;
    long substrLen = d-toCutLen;

    if(substrLen==0){
        return 0;
    }else {
        char *substr;
        substr = (char*) malloc((substrLen + 1)*sizeof(char));
        substr[substrLen] = '\0';
        memcpy(substr, dir, substrLen);
        strcpy((*th_data).wDir, substr);
        free(substr);
    }

    return 0;
}


int mkd_cmd(void *thr_data, char* ptr){
    struct thread_data_t *th_data = (struct thread_data_t*)thr_data;
    char *w;
    w= malloc(strlen((*th_data).wDir)*sizeof(char) + strlen(ptr) * sizeof(char)+1);
    strcpy(w, ".");
    strcat(w, (*th_data).wDir);
    strcat(w, "/");
    strcat(w, ptr);
    int err = mkdir(w, 0777);
    free(w);

    return err;
}
int dele_cmd(void *thr_data, char* ptr) {
    struct thread_data_t *th_data = (struct thread_data_t *) thr_data;
    char *filename;
    filename = calloc(strlen((*th_data).wDir) + strlen(ptr) + 2, sizeof(char));
    strcpy(filename, ".");
    strcat(filename, (*th_data).wDir);
    strcat(filename, "/");
    strcat(filename, ptr);
    int err = remove(filename);
    free(filename);
    return err;
}

int transformPortNumber(char p1[], char p2[]){
    int p1_int = (int)strtol(p1, NULL, 10);
    int p2_int = (int)strtol(p2, NULL, 10);
    char hex1[3];
    char hex2[3];
    char hex[6];
    sprintf(hex1, "%x", p1_int);
    puts(hex1);
    sprintf(hex2, "%x", p2_int);
    puts(hex2);
    sprintf(hex, "%s%s", hex1, hex2);
    int port = (int)strtol(hex, NULL, 16);
    return port;
}
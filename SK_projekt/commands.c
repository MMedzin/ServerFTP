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


int user_cmd(void *thr_data, char* args){
    struct thread_data_t *th_data = (struct thread_data_t*)thr_data;
    char *dir_name = args;
    mkdir(dir_name, 0777);
    (*th_data).username = USER;
    strcpy((*th_data).working_directory, "/");
    strcat((*th_data).working_directory, dir_name);
    printf("USER correct, need pass: 331 ->\n");
    return 0;
}

int stor_cmd(void *thr_data, char *args) {

//            char** nameParts = malloc(50*sizeof(char*));
    struct thread_data_t *th_data = (struct thread_data_t*)thr_data;
    char * filename;
    filename = calloc(strlen((*th_data).working_directory) + strlen(args) + 2, sizeof(char));
    strcpy(filename, ".");
    strcat(filename, (*th_data).working_directory);
    strcat(filename, "/");
    strcat(filename, args);
    printf("Nazwa pliku: %s\n", filename);


    printf("TRYB PRZESYŁANIA W FUNKCJI: %d\n", (*th_data).transfer_mode);
    FILE *fp;
    char buffer[BUF_SIZE];
    char* big_buffer=malloc(BUF_SIZE*sizeof(char));

    if((*th_data).transfer_mode==1) {
        fp = fopen(filename, "w");
    }else if((*th_data).transfer_mode==2){
        fp = fopen(filename, "wb");
    }else{
        printf("Invalid data type: %d\n", (*th_data).transfer_mode);
        return -1;
    }


    if(ferror(fp)!=0){
        printf("Cannot open file: %s\r\n", filename);
        free(filename);
        return -1;
    }
    // TODO dodaj tu coś jak się plik źle otworzy
    int rec_bytes;
    int sum = 0;
    int n = 1;
    while(1){
        bzero(buffer, BUF_SIZE);
        rec_bytes = (int) read((*th_data).fd_file_transfer, buffer, BUF_SIZE-1);
        if(rec_bytes<=0){
            break;
        }
        if(sum==0){
            if((*th_data).transfer_mode==1){
                buffer[rec_bytes] = '\0';
                strcpy(big_buffer, buffer);
                sum+=rec_bytes +1;
            }
            else if((*th_data).transfer_mode==2){
                memcpy(big_buffer, buffer, rec_bytes);
                sum+=rec_bytes;
            }
        }else {
            big_buffer = realloc(big_buffer, n*BUF_SIZE*sizeof(char));
            if((*th_data).transfer_mode==1){
                buffer[rec_bytes] = '\0';
                strcat(big_buffer, buffer);
                sum+=rec_bytes +1;
            }
            else if((*th_data).transfer_mode==2){
                memcpy(big_buffer+sum, buffer, rec_bytes);
                sum+=rec_bytes;
            }
        }
        n++;
    }
    // TODO jak coś się źle odczyta
    strcat(big_buffer, "\0");

    fwrite(big_buffer, 1, sum, fp);
    fclose(fp);
    free(filename);
    bzero(buffer, BUF_SIZE);
    bzero(big_buffer, sizeof(big_buffer));

    return 0;
}

int retr_cmd(void *thr_data, char* args){

    struct thread_data_t *th_data = (struct thread_data_t*)thr_data;
    char * filename;
    filename = calloc(strlen((*th_data).working_directory) + strlen(args) + 2, sizeof(char));
    strcpy(filename, ".");
    strcat(filename, (*th_data).working_directory);
    strcat(filename, "/");
    strcat(filename, args);
    printf("Nazwa pliku: %s\n", filename);

    FILE *fp;
    char *buffer;

    if((*th_data).transfer_mode==1){
        fp = fopen(filename, "r");
    }else if((*th_data).transfer_mode==2){
        fp = fopen(filename, "rb");
    }else{
        printf("Invalid data type: %d\n", (*th_data).transfer_mode);
        return -1;
    }

    int size;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buffer = malloc(size);

    if(ferror(fp)!=0){
        printf("Cannot open file: %s\r\n", filename);
        free(filename);
        return -1;
    }
    // TODO dodaj tu coś jak się plik źle otworzy
    //TODO zobaczyć co z tymi buforami kurna

    int n = (int) fread(buffer, sizeof(char), size, fp);

    if(n==-1){
        printf("Error while reading file: %s\r\n", filename);
        free(filename);
        return -1;
    }

    int result = write((*th_data).fd_file_transfer, buffer, n);

    //todo może lepiej??
    if(result <=0){
        return -1;
    }

    close((*th_data).fd_file_transfer);

    fclose(fp);
    free(filename);
    free(buffer);

    return 0;
}


int rmd_cmd(void *thr_data, char *args) {
    struct thread_data_t *th_data = (struct thread_data_t *) thr_data;
    if (args != NULL) {
        char *pathToDir = calloc(strlen((*th_data).working_directory) + strlen(args) + 2, sizeof(char));
        strcat(pathToDir, ".");
        strcat(pathToDir, (*th_data).working_directory);
        strcat(pathToDir, "/");
        strcat(pathToDir, args);
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
    t_data->do_exit = 0;
    t_data->fd = (*th_data).fd;
    t_data->username = (*th_data).username;
    strcpy(t_data->working_directory, (*th_data).working_directory);
    t_data->fd_file_transfer = (*th_data).fd_file_transfer;
    int create_result;
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
    char command[1047] = "/bin/ls -l '.";
    strcat(command, (*th_data).working_directory);
    strcat(command, "'");
    fp = popen(command, "r");
    if(fp == NULL) {
        printf("failed to read directory\n");
    }
    else{
        unsigned long i;
        while (fgets(path, sizeof(path), fp) != NULL){
//            strcat(path, "\r\n");
            i = strlen(path);
            path[i-1] = '\r';
            path[i] = '\n';
            path[i+1] = '\0';
            write((*th_data).fd_file_transfer, path, strlen(path));
        }
        pclose(fp);
    }

    close((*th_data).fd_file_transfer);
    write((*th_data).fd, "226 Transfer complete.\r\n", strlen("226 Transfer complete.\r\n"));
    printf("226 Transfer complete.\r\n");
    free(th_data);
    pthread_exit(NULL);
}

int cdup_cmd(void *thr_data){
    struct thread_data_t *th_data = (struct thread_data_t*)thr_data;
    char * dir = (*th_data).working_directory;
    long d = strlen(dir);
    long i;
    char delim = '/';

    //Znajdujemy pierwsze wystąpienie / w working directory
    for(i=d-1;i>=0;i--){
        if(dir[i]==delim && i != d-1){
            break;
        }
    }
    long toCutLen = d-i;
    long substrLen = d-toCutLen;

    //Jeżeli jesteśmy w /USER, to nie wykonujemy komendy
    if(substrLen==0){
        return -1;
    }else {
        char *substr;
        substr = (char*) malloc((substrLen + 1)*sizeof(char));
        substr[substrLen] = '\0';
        memcpy(substr, dir, (size_t) substrLen);
        strcpy((*th_data).working_directory, substr);
        free(substr);
    }

    return 0;
}


int mkd_cmd(void *thr_data, char* args){
    struct thread_data_t *th_data = (struct thread_data_t*)thr_data;
    char *w;


    w= malloc(strlen((*th_data).working_directory) * sizeof(char) + strlen(args) * sizeof(char) + 1);
    strcpy(w, ".");
    strcat(w, (*th_data).working_directory);
    strcat(w, "/");
    strcat(w, args);
    int err = mkdir(w, 0777);
    free(w);

    return err;
}
int dele_cmd(void *thr_data, char* args) {
    struct thread_data_t *th_data = (struct thread_data_t *) thr_data;
    char *filename;
    filename = calloc(strlen((*th_data).working_directory) + strlen(args) + 2, sizeof(char));
    strcpy(filename, ".");
    strcat(filename, (*th_data).working_directory);
    strcat(filename, "/");
    strcat(filename, args);
    int err = remove(filename);

    if(err==0){
        delete((*th_data).mutex_table, args);
    }
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

int cwd_cmd(void *thr_data, char*args) {
    struct thread_data_t *th_data = (struct thread_data_t *) thr_data;
    printf("NAZWA DIR W CWD: %s", args);
    if (args != NULL) {
        if(strcmp(args, "/")==0){
            return -1;
        }

        if (strcmp(args, "..") == 0) {
            return cdup_cmd(th_data);
        } else {
            if (strchr(args, '/') != NULL) {
                strcpy((*th_data).working_directory, args);
            } else {
                strcat((*th_data).working_directory, "/");
                strcat((*th_data).working_directory, args);
            }
        }
    }else{
        return -1;
    }
    return 0;
}
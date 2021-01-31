#include "commands.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ftw.h>
#include <errno.h>

#define BUF_SIZE 1000

//Komenda, która sprawdza poprawność nazwy użytkownika i tworzy directory o takiej nazwie
int user_cmd(void *thr_data, char *args) {
    struct thread_data_t *th_data = (struct thread_data_t *) thr_data;
    char *dir_name = args;
    if (mkdir(dir_name, 0777) == -1) {
        printf("\"%s\" directory already exists\n", dir_name);
    }
    (*th_data).username = USER;
    strcpy((*th_data).working_directory, "/");
    strcat((*th_data).working_directory, dir_name);
    printf("USER correct, need pass: 331 ->\n");
    return 0;
}

// Komenda, która odbiera plik od klienta i zapisuje go na serwerze;
// funkcja przyjmuje zawartość pliku od klienta i zapisuje ją pod wskazaną ściężką;
// w zależnosci od ustalonego sposobu przesyłania (komenda TYPE) plik jest traktowany jako binarny (tryb IMAGE) lub
// jako tekstowy (tryb ASCII)
int stor_cmd(void *thr_data, char *args) {
    struct thread_data_t *th_data = (struct thread_data_t *) thr_data;
    char *filename;
    filename = calloc(strlen((*th_data).working_directory) + strlen(args) + 3, sizeof(char));
    strcpy(filename, ".");
    strcat(filename, (*th_data).working_directory);
    strcat(filename, "/");
    strcat(filename, args);

    FILE *fp;
    char buffer[BUF_SIZE];
    char *big_buffer = malloc(BUF_SIZE * sizeof(char)); // bufor przechowujacy całość pobieranego pliku

    if ((*th_data).transfer_mode == 1) {
        fp = fopen(filename, "w");
    } else if ((*th_data).transfer_mode == 2) {
        fp = fopen(filename, "wb");
    } else {
        printf("Invalid data type: %d\n", (*th_data).transfer_mode);
        return -1;
    }

    if (fp == NULL || ferror(fp) != 0) {
        printf("Cannot open file: %s\n", filename);
        free(filename);
        free(big_buffer);
        return -1;
    }

    int rec_bytes;
    int sum = 0;
    int n = 1;
    while (1) {
        bzero(buffer, BUF_SIZE);
        rec_bytes = (int) read((*th_data).fd_file_transfer, buffer, BUF_SIZE - 1);
        if (rec_bytes == 0) {
            break;
        } else if (rec_bytes == -1) {
            printf("Error while reading from client...\n");
            free(filename);
            free(big_buffer);
            return -1;
        }
        if (sum == 0) {
            if ((*th_data).transfer_mode == 1) {
                buffer[rec_bytes] = '\0';
                strcpy(big_buffer, buffer);
                sum += rec_bytes + 1;
            } else if ((*th_data).transfer_mode == 2) {
                memcpy(big_buffer, buffer, (size_t) rec_bytes);
                sum += rec_bytes;
            }
        } else {
            // rozmiar big_buffer jest odpowiednio powiększany przed każdym zapisianiem
            big_buffer = realloc(big_buffer, n * BUF_SIZE * sizeof(char));
            if ((*th_data).transfer_mode == 1) {
                buffer[rec_bytes] = '\0';
                strcat(big_buffer, buffer);
                sum += rec_bytes + 1;
            } else if ((*th_data).transfer_mode == 2) {
                memcpy(big_buffer + sum, buffer, (size_t) rec_bytes);
                sum += rec_bytes;
            }
        }
        n++;
    }

    fwrite(big_buffer, 1, (size_t) sum, fp);

    n = fclose(fp);

    free(filename);
    bzero(buffer, BUF_SIZE);
    free(big_buffer);

    if (n != 0) {
        printf("Error while closing the file...\n");
        return -1;
    }

    return 0;
}

// Komenda, która pozwala przesłać plik do klienta;
// funkcja odczytuje wskazany plik i przesyła jego zawartosć do klienta;
// w zależnosci od ustalonego sposobu przesyłania (komenda TYPE) plik jest przesyłany jako binarny (tryb IMAGE) lub
// jako tekstowy (tryb ASCII)
int retr_cmd(void *thr_data, char *args) {

    struct thread_data_t *th_data = (struct thread_data_t *) thr_data;
    char *filename;
    filename = calloc(strlen((*th_data).working_directory) + strlen(args) + 3, sizeof(char));
    strcpy(filename, ".");
    strcat(filename, (*th_data).working_directory);
    strcat(filename, "/");
    strcat(filename, args);
    printf("Nazwa pliku: %s\n", filename);

    FILE *fp;
    char *buffer;

    if ((*th_data).transfer_mode == 1) {
        fp = fopen(filename, "r");
    } else if ((*th_data).transfer_mode == 2) {
        fp = fopen(filename, "rb");
    } else {
        printf("Invalid data type: %d\n", (*th_data).transfer_mode);
        return -1;
    }

    if (fp == NULL || ferror(fp) != 0) {
        printf("Cannot open file: %s\n", filename);
        free(filename);
        return -1;
    }

    int size;
    fseek(fp, 0, SEEK_END);
    size = (int) ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buffer = malloc((size_t) size);


    int n = (int) fread(buffer, sizeof(char), (size_t) size, fp);

    if (n == -1) {
        printf("Error while reading file: %s\n", filename);
        free(filename);
        fclose(fp);
        free(buffer);
        return -1;
    }

    int result = (int) write((*th_data).fd_file_transfer, buffer, (size_t) n);
    if (result == -1) {
        printf("Error while writting to client...\n");
        fclose(fp);
        free(filename);
        free(buffer);
        return -1;
    }

    if (close((*th_data).fd_file_transfer) != 0) {
        fclose(fp);
        free(filename);
        free(buffer);
        return -1;
    }

    n = fclose(fp);
    free(filename);
    free(buffer);

    if (n != 0) {
        printf("Error while closing the file...\n");
        return -1;
    }

    return 0;
}

//Komenda pozwalająca usunąć katalog
int rmd_cmd(void *thr_data, char *args) {
    struct thread_data_t *th_data = (struct thread_data_t *) thr_data;
    char *pathToDir = calloc(strlen((*th_data).working_directory) + strlen(args) + 3, sizeof(char));
    strcpy(pathToDir, ".");
    strcat(pathToDir, (*th_data).working_directory);
    strcat(pathToDir, "/");
    strcat(pathToDir, args);
    printf("Removing %s...\n", pathToDir);

    if (rmdir(pathToDir) != 0) {
        printf("Error while trying to remove directory, error code: %d\n", errno);
        free(pathToDir);
        return -1;
    }
    free(pathToDir);
    return 0;
}

//Funkcja tworząca wątek odpowiedzialny za przesłanie listy folderu
int list_cmd(void *thr_data) {

    struct thread_data_t *th_data = (struct thread_data_t *) thr_data;

    pthread_t thread;
    struct thread_data_t *t_data = malloc(sizeof(struct thread_data_t));
    t_data->do_exit = 0;
    t_data->fd = (*th_data).fd;
    t_data->username = (*th_data).username;
    strcpy(t_data->working_directory, (*th_data).working_directory);
    t_data->fd_file_transfer = (*th_data).fd_file_transfer;
    int create_result;
    create_result = pthread_create(&thread, NULL, send_list, (void *) t_data);
    if (create_result) {
        printf("Error while creating list command thread, error code: %d\n", create_result);
        return -1;
    }
    return 0;
}

// Funkcja definiująca zachowanie wątku odpowiedzialnego za przesłanie listy
// przesyła listę plików wskazanej lokalizacji w systemie do klienta
void *send_list(void *t_data) {

    pthread_detach(pthread_self());
    struct thread_data_t *th_data = (struct thread_data_t *) t_data;

    FILE *fp;
    char path[1035];
    // składanie komendy tekstu komendy ls używanej do pozyskania listy
    char command[1047] = "/bin/ls -l '.";
    strcat(command, (*th_data).working_directory);
    strcat(command, "'");
    fp = popen(command, "r");
    int chk;
    if (fp == NULL || ferror(fp) != 0) {
        printf("Cannot read directory: %s\n", command);
        close((*th_data).fd_file_transfer);
        free(th_data);
        pthread_exit(NULL);
    } else {
        unsigned long i;
        // przesyłanie kolejno wszystkich elementów na liście do klienta
        while (fgets(path, sizeof(path), fp) != NULL) {
            i = strlen(path);
            path[i - 1] = '\r';
            path[i] = '\n';
            path[i + 1] = '\0';
            chk = (int) write((*th_data).fd_file_transfer, path, strlen(path));
            if (chk == -1) {
                printf("Write operation failed (directory listing), error code: %d\n", errno);
                pclose(fp);
                close((*th_data).fd_file_transfer);
                free(th_data);
                pthread_exit(NULL);
            }
        }
        chk = pclose(fp);
        if (chk == -1) {
            printf("Cannot close directory: %s, error code: %d\n", command, errno);
            close((*th_data).fd_file_transfer);
            free(th_data);
            pthread_exit(NULL);
        }
    }

    chk = close((*th_data).fd_file_transfer);
    if (chk != 0) {
        printf("Data connection closing failed, error code: %d\n", errno);
    }
    chk = (int) write((*th_data).fd, "226 Transfer complete.\r\n", strlen("226 Transfer complete.\r\n"));
    if (chk == -1) {
        printf("Write operation failed (ftp transfer complete response), error code: %d", errno);
    } else {
        printf("226 Transfer complete.\r\n");
    }
    free(th_data);
    pthread_exit(NULL);
}

//Funkcja zmieniająca working directory na jedno wyżej (parent directory)
int cdup_cmd(void *thr_data) {
    struct thread_data_t *th_data = (struct thread_data_t *) thr_data;
    char *dir = (*th_data).working_directory;
    long d = strlen(dir);
    long i;
    char delim = '/';

    //Znajdujemy pierwsze wystąpienie / w working directory
    for (i = d - 1; i >= 0; i--) {
        if (dir[i] == delim && i != d - 1) {
            break;
        }
    }
    long toCutLen = d - i;
    long substrLen = d - toCutLen;

    //Jeżeli jesteśmy w /USER, to nie wykonujemy komendy
    if (substrLen == 0) {
        return 0;
    } else {
        //Usuwamy z working directory jeden człon najbardziej na prawo (do znaku /)
        char *substr;
        substr = (char *) malloc((substrLen + 1) * sizeof(char));
        substr[substrLen] = '\0';
        memcpy(substr, dir, (size_t) substrLen);
        strcpy((*th_data).working_directory, substr);
        free(substr);
    }

    return 0;
}

//Funkcja tworząca nowy katalog
int mkd_cmd(void *thr_data, char *args) {
    struct thread_data_t *th_data = (struct thread_data_t *) thr_data;
    char *new_directory;

    new_directory = malloc((strlen((*th_data).working_directory) + strlen(args) + 3) * sizeof(char));
    strcpy(new_directory, ".");
    strcat(new_directory, (*th_data).working_directory);
    strcat(new_directory, "/");
    strcat(new_directory, args);
    int err = mkdir(new_directory, 0777);

    if (err != 0) {
        printf("Error while creating directory: %s\n", new_directory);
        free(new_directory);
        return -1;
    }

    free(new_directory);
    return 0;
}

//Funkcju usuwająca pliki
int dele_cmd(void *thr_data, char *args) {
    struct thread_data_t *th_data = (struct thread_data_t *) thr_data;
    char *filename;
    filename = calloc(strlen((*th_data).working_directory) + strlen(args) + 3, sizeof(char));
    strcpy(filename, ".");
    strcat(filename, (*th_data).working_directory);
    strcat(filename, "/");
    strcat(filename, args);
    int err = remove(filename);

    if (err == 0) {
        delete((*th_data).mutex_table, args);
    }
    free(filename);
    return err;
}

//Funkcja przekształcająca numery portu z komendy PORT
int transform_port_number(char *p1, char *p2) {
    int p1_int = (int) strtol(p1, NULL, 10);
    int p2_int = (int) strtol(p2, NULL, 10);
    char hex1[3];
    char hex2[3];
    char hex[6];
    sprintf(hex1, "%x", p1_int);
    puts(hex1);
    sprintf(hex2, "%x", p2_int);
    puts(hex2);
    sprintf(hex, "%s%s", hex1, hex2);
    int port = (int) strtol(hex, NULL, 16);
    return port;
}

//Funkcja zmieniająca working directory
int cwd_cmd(void *thr_data, char *args) {
    struct thread_data_t *th_data = (struct thread_data_t *) thr_data;
    printf("NAZWA DIR W CWD: %s\n", args);
    if (args != NULL) {
        if (strcmp(args, "/") == 0) {
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
    } else {
        return -1;
    }
    return 0;
}
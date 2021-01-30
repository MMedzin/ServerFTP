#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>

#include "thread_data_t.h"
#include "hashmap_threads.h"
#include "command_parser.h"
#include "thread_data_t.h"
#include "commands.h"

#define QUEUE_SIZE 5
#define BUF_SIZE 1000
#define HASH_MAP_SIZE 1000


// typy reprezentacji
#define ASCII_TYPE 1
#define IMAGE_TYPE 2
#define UNSET_TYPE (-1)

// zmienne globalne potrzebnych danych
int num_of_conns = 0;
int exit_all = 0;

// funkcja nawiązująca połączenie z klientem do przesyłu danych i zwracająca deskryptor połączenia
int create_file_transfer_conn(char *addr, int port) {
    struct sockaddr_in server_addr;

    int conn_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (conn_socket == -1) {
        printf("Socket could not be created.\nErrorcode: %d\n", errno);
    } else {
        memset(&server_addr, 0, sizeof(struct sockaddr_in));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons((uint16_t) port);

        inet_pton(AF_INET, addr, &(server_addr.sin_addr));

        int conn_status = connect(conn_socket, (struct sockaddr *) &server_addr, sizeof(struct sockaddr));
        if (conn_status) {
            printf("Cannot connect to host.\n Error code: %d\n", errno);
            return -1;
        }
    }
    return conn_socket;
}


// funkcja wątku odpowiadającego za wyłączenie serwera po wpisaniu '-1' na konsoli
void *temp_close() {
    pthread_detach(pthread_self());

    char buff_write[BUF_SIZE];
    while (1) {
        fgets(buff_write, BUF_SIZE, stdin);
        if ((buff_write[0] == '-' && buff_write[1] == '1')) break;
    }
    exit_all = 1;
    pthread_exit(NULL);
}


// funkcja zwracająca odpoowiedź na daną komendę, a także wywołująca odpowiednie operacje związane z tą komendą
char *get_response(char *cmd, void *t_data) {
    struct thread_data_t *th_data = (struct thread_data_t *) t_data;

    pthread_mutex_t file_mutex;
    char cmd_copy[BUF_SIZE];
    strcpy(cmd_copy, cmd);
    char delim[] = " \r";
    char file_delim[] = "\r";
    char *saveptr = cmd_copy;
    char *cmd_cut = strtok_r(cmd_copy, delim, &saveptr); // 'wycięcie' nazwy komendy
    char *filepath; // zmienna zawierająca pełnę ścieżkę do pliku, używana przy znjadowaniu odpowiedniego mutexa
    // zmienne użyteczne przy przetważaniu komend
    char *response = malloc(100);
    char host[16];
    char p1[4];
    char p2[4];
    int port_num;
    int result;
    if(exit_all == 1){
        (*th_data).do_exit = 1;
        sprintf(response, "421 Service not available, closing control connection.\r\n");
        return response;
    }
    switch (command_code(cmd_copy)) {
        case (USER_CMD):
            cmd_cut = strtok_r(NULL, delim, &saveptr);
            if (strcmp(cmd_cut, USER) == 0) {
                user_cmd(th_data, cmd_cut);
                sprintf(response, "331 User name okay, need password.\r\n");
                return response;
            } else {
                printf("Wrong USER: 530 ->\n");
                sprintf(response, "530 Not logged in.\r\n");
                return response;
            }

        case (PASS_CMD):
            cmd_cut = strtok_r(NULL, delim, &saveptr);
            if (strcmp(cmd_cut, PASS) == 0) {
                printf("PASS correct. user logged in: 230 ->\n");
                sprintf(response, "230 User logged in, proceed.\r\n");
                return response;
            } else {
                printf("Wrong PASS: 530 ->\n");
                sprintf(response, "530 Not logged in.\r\n");
                return response;
            }

        case (SYST_CMD):
            sprintf(response, "215 UNIX system type.\n");
            return response;

        case (PWD_CMD):
            sprintf(response, "257 \"%s\" created\r\n", (*th_data).working_directory);
            return response;

        case (TYPE_CMD):
            cmd_cut = strtok_r(NULL, delim, &saveptr);
            if (strcmp(cmd_cut, "A") == 0) {
                (*th_data).transfer_mode = ASCII_TYPE;
                sprintf(response, "200 Command okay.\r\n");
                return response;
            } else if (strcmp(cmd_cut, "I") == 0) {
                (*th_data).transfer_mode = IMAGE_TYPE;
                sprintf(response, "200 Command okay.\r\n");
                return response;
            }
            sprintf(response, "504 Command not implemented for that parameter.\r\n");
            return response;

        case (PORT_CMD):
            for (int i = 0; i < 4; i++) {
                cmd_cut = strtok_r(NULL, ",", &saveptr);
                if (cmd_cut != NULL) {
                    for (int j = 0; j < strlen(cmd_cut); j++) {
                        host[j] = *(cmd_cut + j);
                        if (j == strlen(cmd_cut) - 1) {
                            if (i != 3) host[j + 1] = '.';
                            else host[j + 1] = '\0';
                        }
                    }
                } else {
                    break;
                }
            }
            printf("\n HOST: %s\n", host);
            cmd_cut = strtok_r(NULL, ",", &saveptr);
            if (cmd_cut != NULL) {
                for (int i = 0; i < 3; i++) {
                    p1[i] = *(cmd_cut + i);
                    if (*(cmd_cut + i) == '\0') break;
                }
                p1[3] = '\0';
                cmd_cut = strtok_r(NULL, delim, &saveptr);
                if (cmd_cut != NULL) {
                    for (int i = 0; i < 3; i++) {
                        p2[i] = *(cmd_cut + i);
                        if (*(cmd_cut + i) == '\0') break;
                    }
                    p2[3] = '\0';
                    port_num = transform_port_number(p1, p2);
                    free((*th_data).file_transfer_address);
                    (*th_data).file_transfer_address = malloc(sizeof(host));
                    strcpy((*th_data).file_transfer_address, host);
                    (*th_data).file_transfer_port = port_num;
                    sprintf(response, "200 Command okay.\r\n");
                    return response;
                }
                sprintf(response, "504 Command not implemented for that parameter.\r\n");
                return response;
            }

            if (cmd_cut != NULL && strlen(cmd_cut) == 3) {
                for (int j = 0; j < 3; j++) {
                    host[j] = cmd_cut[j];
                }
                host[4] = '.';
                sprintf(response, "200 Command okay.\r\n");
                return response;
            }
            sprintf(response, "504 Command not implemented for that parameter.\r\n");
            return response;

        case (LIST_CMD):
            (*th_data).fd_file_transfer = create_file_transfer_conn((*th_data).file_transfer_address,
                                                                    (*th_data).file_transfer_port);
            if ((*th_data).fd_file_transfer == -1) {
                sprintf(response, "425 Can't open data connection.\r\n");
                return response;
            }

            result = list_cmd(th_data);

            if (result == 0) {
                sprintf(response, "150 Opening data connection for file list.\r\n");
                return response;
            } else {
                sprintf(response, "500 Syntax error, command unrecognized.\r\n");
                return response;
            }

        case (QUIT_CMD):
            (*th_data).do_exit = 1;
            sprintf(response, "221 Service closing control connection.\r\n");
            return response;
        case (RMD_CMD):
            cmd_cut = strtok_r(NULL, file_delim, &saveptr);
            result = rmd_cmd(th_data, cmd_cut);
            if (result == 0) {
                sprintf(response, "250 Directory removed.\r\n");
                return response;
            }
            sprintf(response, "500 Syntax error.\r\n");
            return response;
        case (CWD_CMD):
            cmd_cut = strtok_r(NULL, file_delim, &saveptr);

            if (cmd_cut == NULL) {
                sprintf(response, "501 Syntax error in parameters or arguments.\r\n");
                return response;
            }

            result = cwd_cmd(t_data, cmd_cut);

            if (result == 0) {
                sprintf(response, "200 working directory changed.\r\n");
                return response;
            }
            sprintf(response, "500 Syntax error.\r\n");
            return response;
        case (CDUP_CMD):
            result = cdup_cmd(th_data);
            if (result == 0) {
                sprintf(response, "200 working directory changed.\r\n");
                return response;
            }
            sprintf(response, "500 Syntax error.\r\n");
            return response;
        case (MKD_CMD):
            cmd_cut = strtok_r(NULL, file_delim, &saveptr);
            if (cmd_cut == NULL) {
                sprintf(response, "501 Syntax error in parameters or arguments.\r\n");
                return response;
            }

            result = mkd_cmd(th_data, cmd_cut);

            if (result == 0) {
                sprintf(response, "257 \"%s\" created\r\n", cmd_cut);
                return response;
            }
            sprintf(response, "501 Syntax error in parameters or arguments.\r\n");
            return response;


        case (STOR_CMD):
            //Utworzenie połączenia, przez które zostanie odebrany plik
            (*th_data).fd_file_transfer = create_file_transfer_conn((*th_data).file_transfer_address,
                                                                    (*th_data).file_transfer_port);
            if ((*th_data).fd_file_transfer == -1) {
                sprintf(response, "425 Can't open data connection.\r\n");
                return response;
            } else {
                write((th_data)->fd, "150 Opening data connection for file list.\r\n",
                      strlen("150 Opening ASCII mode data connection for file list.\r\n"));
            }

            cmd_cut = strtok_r(NULL, file_delim, &saveptr);

            if (cmd_cut == NULL) {
                sprintf(response, "501 Syntax error in parameters or arguments.\r\n");
                return response;
            }

            filepath = malloc(sizeof((*th_data).working_directory)+sizeof(cmd_cut));
            strcpy(filepath, (*th_data).working_directory);
            strcat(filepath, cmd_cut);

            file_mutex = lookup((th_data)->mutex_table, filepath);
            pthread_mutex_lock(&file_mutex);

            result = stor_cmd(th_data, cmd_cut);
            pthread_mutex_unlock(&file_mutex);

            free(filepath);

            if (result == 0) {
                sprintf(response, "250 Requested file action successful.\r\n");
                return response;
            }
            sprintf(response, "552 Requested file action aborted.\r\n");
            return response;

        case DELE_CMD:
            cmd_cut = strtok_r(NULL, file_delim, &saveptr);

            file_mutex = lookup((th_data)->mutex_table, cmd_cut);
            pthread_mutex_lock(&file_mutex);

            result = dele_cmd(th_data, cmd_cut);

            pthread_mutex_unlock(&file_mutex);
            if (cmd_cut == NULL) {
                sprintf(response, "501 Syntax error in parameters or arguments.\r\n");
                return response;
            }
            if (result != 0) {
                sprintf(response, "550 Requested action not taken.\r\n");
                return response;
            }
            sprintf(response, "250 Requested file action okay, completed.\r\n");
            return response;
        case RETR_CMD:
            //Utworzenie połączenia, przez które zostanie odebrany plik
            (*th_data).fd_file_transfer = create_file_transfer_conn((*th_data).file_transfer_address,
                                                                    (*th_data).file_transfer_port);
            if ((*th_data).fd_file_transfer == -1) {
                sprintf(response, "425 Can't open data connection.\r\n");
                return response;
            } else {
                write((th_data)->fd, "150 Opening data connection for file list.\r\n",
                      strlen("150 Opening ASCII mode data connection for file list.\r\n"));
            }
            cmd_cut = strtok_r(NULL, file_delim, &saveptr);

            if (cmd_cut == NULL) {
                sprintf(response, "501 Syntax error in parameters or arguments.\r\n");
                return response;
            }
            filepath = malloc(sizeof((*th_data).working_directory)+sizeof(cmd_cut));
            strcpy(filepath, (*th_data).working_directory);
            strcat(filepath, cmd_cut);

            file_mutex = lookup((th_data)->mutex_table, filepath);
            pthread_mutex_lock(&file_mutex);

            result = retr_cmd(t_data, cmd_cut);

            pthread_mutex_unlock(&file_mutex);
            free(filepath);
            if (result == 0) {
                sprintf(response, "250 Requested file action successful.\r\n");
                return response;
            }
            sprintf(response, "552 Requested file action aborted.\r\n");
            return response;

        default:
            printf("Wrong cmd: 500 ->\n");
            sprintf(response, "500 Syntax error, command unrecognized.\r\n");
            return response;
    }
}

// funkcja opisującą zachowanie głównego wątku tworzonego dla każdego klienta
// - musi przyjmować argument typu (void *) i zwracać (void *)
void *thread_behavior(void *t_data) {
    pthread_detach(pthread_self());
    struct thread_data_t *th_data = (struct thread_data_t *) t_data;

    char *buff_write;
    char buff_read[BUF_SIZE];
    char in[1];
    int r, sum_r;
    int cr_found, nl_found;
    int conn_closed = 0;
    while (1) {
        cr_found = 0;
        nl_found = 0;
        sum_r = 0;
        while (cr_found == 0 || nl_found == 0) {
            r = (int) read((*th_data).fd, in, sizeof(in));
            if (r >= 0) {
                if (r == 0) {
                    conn_closed = 1;
                    break;
                } else {
                    buff_read[sum_r] = in[0];
                    if (cr_found != 0) {
                        if (in[0] == '\n') {
                            nl_found = 1;
                            buff_read[sum_r + 1] = '\0';
                        } else cr_found = 0;
                    }
                    if (in[0] == '\r') {
                        cr_found = 1;
                    }
                    sum_r++;
                }
            } else {
                printf("Reading operation failed, error code: %d\n", errno);
            }
        }
        if (conn_closed != 0 || (*th_data).do_exit == 1) break;
        printf("%s", buff_read);
        buff_write = get_response(buff_read, th_data);
        printf("%s", buff_write);
        write((*th_data).fd, buff_write, strlen(buff_write));
        free(buff_write);
    }
    printf("Quitting...");
    (*th_data).do_exit = 1;

    close((*th_data).fd_file_transfer);
    close((*th_data).fd);

    //clearTable(th_data->mutex_table);
    free((*th_data).file_transfer_address);
    free(th_data);
    num_of_conns--;
    printf(" done\n");
    pthread_join(pthread_self(), NULL);
    pthread_exit(NULL);
}

// funkcja obsługująca połączenie z nowym klientem
void handle_connection(int connection_socket_descriptor, struct table *mutex_table) {
    int create_result;

    //uchwyt na wątek
    pthread_t thread1;

    //dane, które zostaną przekazane do wątku
    //dynamiczne utworzenie instancji struktury thread_data_t o nazwie t_data (+ w odpowiednim miejscu zwolnienie pamięci)
    struct thread_data_t *t_data = malloc(sizeof(struct thread_data_t));
    //wypełnienie pól struktury
    t_data->do_exit = 0;
    t_data->fd = connection_socket_descriptor;
    t_data->fd_file_transfer = 0;
    t_data->mutex_table = mutex_table;
    t_data->file_transfer_address = malloc(1);

    create_result = pthread_create(&thread1, NULL, thread_behavior, (void *) t_data);
    if (create_result) {
        printf("Conection thread creation failed, error code: %d\n", create_result);
        exit(-1);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Invalid arguments. Correct use: ./server <IPv4 ADDR> <PORT NUMBER>\n");
        exit(-1);
    }
    int port_num = atoi(argv[2]);

    if (port_num <= 0) {
        printf("Invalid port number!\n");
        exit(-1);
    }

    struct table *mutex_table = createTable(HASH_MAP_SIZE);


    int server_socket_descriptor;
    int connection_socket_descriptor;
    int bind_result;
    int listen_result;
    char reuse_addr_val = 1;
    struct sockaddr_in server_address;
    printf("Passed parameters: %s %s\n", argv[1], argv[2]);

    //inicjalizacja gniazda serwera

    memset(&server_address, 0, sizeof(struct sockaddr));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &(server_address.sin_addr)) <= 0) {
        printf("Invalid IP address!\n");
        exit(-1);
    }

    server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_descriptor < 0) {
        fprintf(stderr, "%s: Socket creation failed.\n", argv[0]);
        exit(1);
    }
    int flags;
    flags = fcntl(server_socket_descriptor,F_GETFL,0);
    if(flags == -1){
        printf("Obtaining server socket flags failed.\n");
        exit(-1);
    }
    fcntl(server_socket_descriptor, F_SETFL, flags | O_NONBLOCK);
    setsockopt(server_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_val, sizeof(reuse_addr_val));

    bind_result = bind(server_socket_descriptor, (struct sockaddr *) &server_address, sizeof(struct sockaddr));
    if (bind_result < 0) {
        fprintf(stderr, "%s: Attaching IP address and port number to socket failed.\n", argv[0]);
        exit(1);
    }

    listen_result = listen(server_socket_descriptor, QUEUE_SIZE);
    if (listen_result < 0) {
        fprintf(stderr, "%s: Setting queue size failed.\n", argv[0]);
        exit(1);
    }


    pthread_t tempThread;
    pthread_create(&tempThread, NULL, temp_close, NULL);

    while (exit_all != 1) {
        connection_socket_descriptor = accept(server_socket_descriptor, NULL, NULL);
        if(connection_socket_descriptor >= 0){
            write(connection_socket_descriptor, "220 Service ready for new user.\n",
                  strlen("220 Service ready for new user.\n"));
            num_of_conns++;
            handle_connection(connection_socket_descriptor, mutex_table);
        }
    }

    while(num_of_conns>0);

    pthread_join(tempThread, NULL);
    close(server_socket_descriptor);
    clearTable(mutex_table);
    return (0);
}
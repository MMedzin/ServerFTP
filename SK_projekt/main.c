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

#include "thread_data_t.h"
#include "hashmap_threads.h"
#include "command_parser.h"
#include "thread_data_t.h"
#include "commands.h"

#define QUEUE_SIZE 5
#define BUF_SIZE 1000
#define HASH_MAP_SIZE 1000


// typy reprezentacji -- na razie tylko zapisywane, wspierany jest wyłącznie tryb ASCII
#define ASCII_TYPE 1
#define IMAGE_TYPE 2
#define UNSET_TYPE (-1)

// zmienne globalne potrzebnych danych
// TODO - pozbyć się tych zmiennych (singleton)
int num_of_conns = 0;
int exitAll = 0;


//struktura zawierająca dane, które zostaną przekazane do wątku


// funkcja nawiązująca połączenie z klientem do przesyłu danych i zwracająca deskryptor połączenia
int createFileTransferConn(char addr[], int port) {
    struct sockaddr_in server_addr;

    int connSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (connSocket == -1) {
        printf("Socket could not be created.\nErrorcode: %d\n", errno);
    } else {
        memset(&server_addr, 0, sizeof(struct sockaddr_in));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons((uint16_t) port);

        inet_pton(AF_INET, addr, &(server_addr.sin_addr));

        int connStatus = connect(connSocket, (struct sockaddr *) &server_addr, sizeof(struct sockaddr));
        if (connStatus) {
            printf("Cannot connect to host.\n Error code: %d\n", errno);
            return -1;
        }
    }
    return connSocket;
}


// funkcja wątku odpowiadającego za wyłączenie serwera po wpisaniu '-1' na konsoli
void *tempClose(void *th_data) {
    pthread_detach(pthread_self());

    char buff_write[BUF_SIZE];
    while (1) {
        fgets(buff_write, BUF_SIZE, stdin);
        if ((buff_write[0] == '-' && buff_write[1] == '1')) break;
    }
    exitAll = 1;
    pthread_exit(NULL);
}


// funkcja rozpoznająca przychodzące komendy ftp i zwracająca odpowidający im kod

// funkcja zwracająca odpoowiedź na daną komendę, a także wywołująca odpowiednie operacje związane z tą komendą
char *getResponse(char *cmd, void *t_data) {
    struct thread_data_t *th_data = (struct thread_data_t *) t_data;

    pthread_mutex_t file_mutex;
    char cmdCopy[BUF_SIZE];
    strcpy(cmdCopy, cmd);
    char delim[] = " \r";
    char file_delim[] = "\r";
    char *saveptr = cmdCopy;
    char *cmd_cut = strtok_r(cmdCopy, delim, &saveptr); // 'wycięcie' nazwy komendy
    // zmienne użyteczne przy przetważaniu komend
    char *response = malloc(100);
    char host[16];
    char p1[4];
    char p2[4];
    int portNum;
    int result;
    int increment = 0;
    switch (commandCode(cmdCopy)) {
        case (USER_CMD):
            cmd_cut = strtok_r(NULL, delim, &saveptr);
            if (strcmp(cmd_cut, USER) == 0) {
                user_cmd(th_data, cmd_cut);
                return "331 User name okay, need password.\r\n";
            } else {
                printf("Wrong USER: 530 ->\n");
                return "530 Not logged in.\r\n";
            }

        case (PASS_CMD):
            cmd_cut = strtok_r(NULL, delim, &saveptr);
            if (strcmp(cmd_cut, PASS) == 0) {
                printf("PASS correct. user logged in: 230 ->\n");
                return "230 User logged in, proceed.\r\n";
            } else {
                printf("Wrong PASS: 530 ->\n");
                return "530 Not logged in.\r\n";
            }

        case (SYST_CMD):
            return "215 UNIX system type.\n";

        case (PWD_CMD):
            sprintf(response, "257 \"%s\" created\r\n", (*th_data).working_directory);
            return response;

        case (TYPE_CMD):
            cmd_cut = strtok_r(NULL, delim, &saveptr);
            if (strcmp(cmd_cut, "A") == 0) {
                (*th_data).transfer_mode = ASCII_TYPE;
                return "200 Command okay.\r\n";
            } else if (strcmp(cmd_cut, "I") == 0) {
                (*th_data).transfer_mode = IMAGE_TYPE;
                return "200 Command okay.\r\n";
            }
            return "504 Command not implemented for that parameter.\r\n";

        case (PORT_CMD):
            //TODO może lepsze napisanie tej komendy port
            for (int i = 0; i < 4; i++) {
                cmd_cut = strtok_r(NULL, ",", &saveptr);
                if (cmd_cut != NULL) {
                    for (int j = 0; j < strlen(cmd_cut); j++) {
                        host[j + increment] = *(cmd_cut + j);
                        if (j == strlen(cmd_cut) - 1) {
                            if (i != 3) host[j + increment + 1] = '.';
                            else host[j + increment + 1] = '\0';
                        }
                    }
                    printf("\n HOST: %s\n", host);
                    increment += (int) strlen(cmd_cut) + 1;
                } else {
                    break;
                }
            }
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
                    portNum = transformPortNumber(p1, p2);
                    free((*th_data).file_transfer_address);
                    (*th_data).file_transfer_address = malloc(sizeof(host));
                    strcpy((*th_data).file_transfer_address, host);
                    (*th_data).file_transfer_port = portNum;
                    return "200 Command okay.\r\n";
                }
                return "504 Command not implemented for that parameter.\r\n";
            }

            if (cmd_cut != NULL && strlen(cmd_cut) == 3) {
                for (int j = 0; j < 3; j++) {
                    host[j] = cmd_cut[j];
                }
                host[4] = '.';
                cmd_cut = strtok_r(NULL, ",", &saveptr);
                return "200 Command okay.\r\n";
            }
            return "504 Command not implemented for that parameter.\r\n";

        case (LIST_CMD):
            (*th_data).fd_file_transfer = createFileTransferConn((*th_data).file_transfer_address,
                                                                 (*th_data).file_transfer_port);
            if ((*th_data).fd_file_transfer == -1) {
                return "425 Can't open data connection.\r\n";
            }

            result = list_cmd(th_data);

            if (result == 0) {
                return "150 Opening ASCII mode data connection for file list.\r\n";
            } else {
                return "500 Syntax error, command unrecognized.\r\n";
            }

        case (QUIT_CMD):
            (*th_data).do_exit = 1;
            return "221 Service closing control connection.\r\n";
        case (RMD_CMD):
            cmd_cut = strtok_r(NULL, delim, &saveptr);
            result = rmd_cmd(th_data, cmd_cut);
            if (result == 0) {
                return "250 Directory removed.\r\n";
            }

            return "500 Syntax error.\r\n";
        case (CWD_CMD):
            cmd_cut = strtok_r(NULL, file_delim, &saveptr);

            if (cmd_cut == NULL) {
                return "501 Syntax error in parameters or arguments.\r\n";
            }

            result = cwd_cmd(t_data, cmd_cut);

            if (result == 0) {
                return "200 working directory changed.\r\n";
            }

            return "500 Syntax error.\r\n";
        case (CDUP_CMD):
            result = cdup_cmd(th_data);
            if (result == 0) {
                return "200 working directory changed.\r\n";
            }

            return "500 Syntax error.\r\n";
        case (MKD_CMD):
            cmd_cut = strtok_r(NULL, file_delim, &saveptr);
            if (cmd_cut == NULL) {
                return "501 Syntax error in parameters or arguments.\r\n";
            }

            result = mkd_cmd(th_data, cmd_cut);

            if (result == 0) {
                sprintf(response, "257 \"%s\" created\r\n", cmd_cut);
                return response;
            }
            return "501 Syntax error in parameters or arguments.\r\n";


        case (STOR_CMD):
            //Utworzenie połączenia, przez które zostanie odebrany plik
            (*th_data).fd_file_transfer = createFileTransferConn((*th_data).file_transfer_address,
                                                                 (*th_data).file_transfer_port);
            if ((*th_data).fd_file_transfer == -1) {
                return "425 Can't open data connection.\r\n";
            } else {
                write((th_data)->fd, "150 Opening ASCII mode data connection for file list.\r\n",
                      strlen("150 Opening ASCII mode data connection for file list.\r\n"));
            }

            cmd_cut = strtok_r(NULL, file_delim, &saveptr);

            if (cmd_cut == NULL) {
                return "501 Syntax error in parameters or arguments.\r\n";
            }

            file_mutex = lookup((th_data)->mutex_table, cmd_cut);
            pthread_mutex_lock(&file_mutex);

            result = stor_cmd(th_data, cmd_cut);
            pthread_mutex_unlock(&file_mutex);


            if (result == 0) {
                return "250 Requested file action successful.\r\n";
            }
            return "552 Requested file action aborted.\r\n";

        case DELE_CMD:
            cmd_cut = strtok_r(NULL, file_delim, &saveptr);

            file_mutex = lookup((th_data)->mutex_table, cmd_cut);
            pthread_mutex_lock(&file_mutex);

            result = dele_cmd(th_data, cmd_cut);

            pthread_mutex_unlock(&file_mutex);
            if (cmd_cut == NULL) {
                return "501 Syntax error in parameters or arguments.\r\n";
            }
            if (result != 0) {
                return "550 Requested action not taken.\r\n";
            }

            return "250 Requested file action okay, completed.\r\n";
        case RETR_CMD:
            //Utworzenie połączenia, przez które zostanie odebrany plik
            (*th_data).fd_file_transfer = createFileTransferConn((*th_data).file_transfer_address,
                                                                 (*th_data).file_transfer_port);
            if ((*th_data).fd_file_transfer == -1) {
                return "425 Can't open data connection.\r\n";
            } else {
                write((th_data)->fd, "150 Opening ASCII mode data connection for file list.\r\n",
                      strlen("150 Opening ASCII mode data connection for file list.\r\n"));
            }
            cmd_cut = strtok_r(NULL, file_delim, &saveptr);

            if (cmd_cut == NULL) {
                return "501 Syntax error in parameters or arguments.\r\n";
            }
            file_mutex = lookup((th_data)->mutex_table, cmd_cut);
            pthread_mutex_lock(&file_mutex);

            result = retr_cmd(t_data, cmd_cut);

            pthread_mutex_unlock(&file_mutex);
            if (result == 0) {
                return "250 Requested file action successful.\r\n";
            }
            return "552 Requested file action aborted.\r\n";

        default:
            printf("Wrong cmd: 500 ->\n");
            return "500 Syntax error, command unrecognized.\r\n";
    }
}

//funkcja opisującą zachowanie wątku - musi przyjmować argument typu (void *) i zwracać (void *)
void *ThreadBehavior(void *t_data) {
    pthread_detach(pthread_self());
    struct thread_data_t *th_data = (struct thread_data_t *) t_data;

    char *buff_write;
    char buff_read[BUF_SIZE];
    char in[1];
    int r, sumR;
    int cr_found, nl_found;
    int conn_closed = 0;
    while (1) {
        cr_found = 0;
        nl_found = 0;
        sumR = 0;
        while (cr_found == 0 || nl_found == 0) {
            r = (int) read((*th_data).fd, in, sizeof(in));
            if (r >= 0) {
                if (r == 0) {
                    conn_closed = 1;
                    break;
                } else {
                    buff_read[sumR] = in[0];
                    if (cr_found != 0) {
                        if (in[0] == '\n') {
                            nl_found = 1;
                            buff_read[sumR + 1] = '\0';
                        } else cr_found = 0;
                    }
                    if (in[0] == '\r') {
                        cr_found = 1;
                    }
                    sumR++;
                }
            } else {
                printf("Wystąpił błąd odczytu, kod błędu: %d", errno);
            }
        }
        if (conn_closed != 0 || (*th_data).do_exit == 1) break;
        printf("%s", buff_read);
        buff_write = getResponse(buff_read, th_data);
        printf("%s", buff_write);
        write((*th_data).fd, buff_write, strlen(buff_write));
    }
    printf("Quitting...\n");
    (*th_data).do_exit = 1;

    close((*th_data).fd_file_transfer);

    clearTable(th_data->mutex_table);
    free(th_data);
    num_of_conns--;
    pthread_exit(NULL);
}

//funkcja obsługująca połączenie z nowym klientem
void handleConnection(int connection_socket_descriptor, struct table *mutex_table) {
    int create_result = 0;

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

    create_result = pthread_create(&thread1, NULL, ThreadBehavior, (void *) t_data);
    if (create_result) {
        printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
        exit(-1);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Invalid arguments. Correct use: ./server <IPv4 ADDR> <PORT NUMBER>\n\r");
        exit(-1);
    }
    int port_num = atoi(argv[2]);

    if (port_num <= 0) {
        printf("Invalid port number!\n\r");
        exit(-1);
    }

    struct table *mutex_table = createTable(HASH_MAP_SIZE);


    int server_socket_descriptor;
    int connection_socket_descriptor;
    int bind_result;
    int listen_result;
    char reuse_addr_val = 1;
    struct sockaddr_in server_address;
    printf("Arguemnty %s", argv[1]);

    //inicjalizacja gniazda serwera

    memset(&server_address, 0, sizeof(struct sockaddr));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &(server_address.sin_addr)) <= 0) {
        printf("Invalid IP address!\n\r");
        exit(-1);
    }

    server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_descriptor < 0) {
        fprintf(stderr, "%s: Błąd przy próbie utworzenia gniazda..\n", argv[0]);
        exit(1);
    }
    setsockopt(server_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_val, sizeof(reuse_addr_val));

    bind_result = bind(server_socket_descriptor, (struct sockaddr *) &server_address, sizeof(struct sockaddr));
    if (bind_result < 0) {
        fprintf(stderr, "%s: Błąd przy próbie dowiązania adresu IP i numeru portu do gniazda.\n", argv[0]);
        exit(1);
    }

    listen_result = listen(server_socket_descriptor, QUEUE_SIZE);
    if (listen_result < 0) {
        fprintf(stderr, "%s: Błąd przy próbie ustawienia wielkości kolejki.\n", argv[0]);
        exit(1);
    }


    pthread_t tempThread;
    pthread_create(&tempThread, NULL, tempClose, NULL);

    while (exitAll != 1) {
        connection_socket_descriptor = accept(server_socket_descriptor, NULL, NULL);
        write(connection_socket_descriptor, "220 Service ready for new user.\n",
              strlen("220 Service ready for new user.\n"));
        if (connection_socket_descriptor < 0) {
            fprintf(stderr, "%s: Błąd przy próbie utworzenia gniazda dla połączenia.\n", argv[0]);
            exit(1);
        }
        num_of_conns++;
        handleConnection(connection_socket_descriptor, mutex_table);
    }


    close(server_socket_descriptor);
    return (0);
}
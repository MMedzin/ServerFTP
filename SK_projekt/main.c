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
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <dirent.h>
#include <ftw.h>
#include "thread_data_t.h"

#include "utils.h"
#include "command_parser.h"
#include "thread_data_t.h"
#include "commands.h"

#define SERVER_PORT 1235
#define QUEUE_SIZE 5
#define BUF_SIZE 1000
#define CON_LIMIT 3



// typy reprezentacji -- na razie tylko zapisywane, wspierany jest wyłącznie tryb ASCII
#define ASCII_TYPE 1
#define IMAGE_TYPE 2
#define UNSET_TYPE (-1)

// zmienne globalne potrzebnych danych
// TODO - pozbyć się tych zmiennych (singleton)
int num_of_conns = 0;
int exitAll = 0;
int rep_type = UNSET_TYPE;

//struktura zawierająca dane, które zostaną przekazane do wątku


// funkcja nawiązująca połączenie z klientem do przesyłu danych i zwracająca deskryptor połączenia
int createFileTransferConn(char addr[], int port)
{
    struct sockaddr_in server_addr;

    int connSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(connSocket==-1){
        printf("Socket could not be created.\nErrorcode: %d\n", errno);
    }
    else{
        memset(&server_addr, 0, sizeof(struct sockaddr_in));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);

        inet_pton(AF_INET, addr, &(server_addr.sin_addr));

        int connStatus = connect(connSocket, (struct sockaddr*) &server_addr, sizeof(struct sockaddr));
        if(connStatus){
            printf("Cannot connect to host.\n Error code: %d\n", errno);
            return -1;
        }
    }
    return connSocket;
}


// funkcja wątku odpowiadającego za wyłączenie serwera po wpisaniu '-1' na konsoli
void *tempClose(void *th_data)
{
    pthread_detach(pthread_self());

    char buff_write[BUF_SIZE];
    while(1){
        fgets(buff_write, BUF_SIZE, stdin);
        if((buff_write[0] == '-' && buff_write[1] == '1')) break;
    }
    exitAll = 1;
    pthread_exit(NULL);
}







// funkcja rozpoznająca przychodzące komendy ftp i zwracająca odpowidający im kod

// funkcja zwracająca odpoowiedź na daną komendę, a także wywołująca odpowiednie operacje związane z tą komendą
char* getResponse(char* cmd, void *t_data)
{
    struct thread_data_t *th_data = (struct thread_data_t*)t_data;

    char cmdCopy[BUF_SIZE];
    strcpy(cmdCopy, cmd);
    char delim[] = " \r";
    char * saveptr;
    char* ptr = strtok_r(cmdCopy, delim, &saveptr); // 'wycięcie' nazwy komendy
    // zmienne użyteczne przy przetważaniu komend
    char* response = malloc(100);
    char host[16];
    char p1[4];
    char p2[4];
    char *filename;
    int portNum;
    int result;
    int increment = 0; // zmienna używana przy komendzie TYPE do odpowiedniego przepisania adresu hosta
    switch (commandCode(cmdCopy))
    {
        case (USER_CMD):
            ptr = strtok_r(NULL, delim, &saveptr);
            if(strcmp(ptr, USER) == 0){
            user_cmd(th_data, ptr);
                return "331 User name okay, need password.\r\n";
            }
            else{
                printf("Wrong USER: 530 ->\n");
                return "530 Not logged in.\r\n";
            }
            break;

        case (PASS_CMD):
            ptr = strtok_r(NULL, delim, &saveptr);
            if(strcmp(ptr, PASS) == 0){
                printf("PASS correct. user logged in: 230 ->\n");
                return "230 User logged in, proceed.\r\n";
            }
            else{
                printf("Wrong PASS: 530 ->\n");
                return "530 Not logged in.\r\n";
            }
            break;

        case (SYST_CMD):
            return "215 UNIX system type.\n";

        case (PWD_CMD):
            sprintf(response, "257 \"%s\" created\r\n", (*th_data).wDir);
            return response;

        case (TYPE_CMD):
            ptr = strtok_r(NULL, delim, &saveptr);
            if(strcmp(ptr, "A") == 0){
                (*th_data).transferMode = ASCII_TYPE;
                return "200 Command okay.\r\n";
            }
            else if(strcmp(ptr, "I") == 0){
                (*th_data).transferMode = IMAGE_TYPE;
                return "200 Command okay.\r\n";
            }
            return "504 Command not implemented for that parameter.\r\n";

        case (PORT_CMD):
            for(int i = 0; i < 4; i++){
                ptr = strtok_r(NULL, ",", &saveptr);
                if(ptr!=NULL){
                    for(int j = 0; j<strlen(ptr); j++){
                        host[j+increment] = *(ptr+j);
                        if(j==strlen(ptr)-1){
                            if(i!=3) host[j+increment+1]='.';
                            else host[j+increment+1]='\0';
                        }
                    }
                    printf("\n HOST: %s\n", host);
                    increment += (int)strlen(ptr) + 1;
                }
                else{
                    break;
                }
            }
            ptr = strtok_r(NULL, ",", &saveptr);
            if(ptr!=NULL){
                for(int i = 0; i<3; i++){
                    p1[i] = *(ptr+i);
                    if(*(ptr+i)=='\0') break;
                }
                p1[3] = '\0';
                ptr = strtok_r(NULL, delim, &saveptr);
                if(ptr!=NULL){
                    for(int i = 0; i<3; i++){
                        p2[i] = *(ptr+i);
                        if(*(ptr+i)=='\0') break;
                    }
                    p2[3] = '\0';
//                    if((*th_data).fileTransferConn != 0) close((*th_data).fileTransferConn);
                    portNum = transformPortNumber(p1, p2);
                    free((*th_data).fileTransferAddress);
                    (*th_data).fileTransferAddress = malloc(sizeof(host));
                    strcpy((*th_data).fileTransferAddress, host);
                    (*th_data).fileTransferPort = portNum;
                    return "200 Command okay.\r\n";
                }
                return "504 Command not implemented for that parameter.\r\n";
            }

            if(ptr!=NULL && strlen(ptr) == 3){
                for(int j = 0; j<3; j++){
                    host[j] = ptr[j];
                }
                host[4]='.';
                ptr = strtok_r(NULL, ",", &saveptr);
                return "200 Command okay.\r\n";
            }
            return "504 Command not implemented for that parameter.\r\n";

        case (LIST_CMD):
            (*th_data).fileTransferConn = createFileTransferConn((*th_data).fileTransferAddress,
                                                                 (*th_data).fileTransferPort);
            if((*th_data).fileTransferConn==-1){
                return "425 Can't open data connection.\r\n";
            }

            result = list_cmd(th_data);

            if(result==0){
                return "150 Opening ASCII mode data connection for file list.\r\n";
            }
            else{
                return "500 Syntax error, command unrecognized.\r\n";
            }

        case (QUIT_CMD):
            (*th_data).doExit = 1;
            return "221 Service closing control connection.\r\n";
        case (RMD_CMD):
            ptr = strtok_r(NULL, delim, &saveptr);
            result = rmd_cmd(th_data, ptr);
            if(result ==0) {
                return "250 Directory removed.\r\n";
            }

            return "500 Syntax error.\r\n";
        case (CWD_CMD):
            ptr = strtok_r(NULL, delim, &saveptr);
            if(ptr != NULL){
                if(strcmp(ptr, "..") ==0) {
                    cdup_cmd(th_data);
                }
                else{
                    if(strchr(ptr, '/') != NULL){
                        strcpy((*th_data).wDir, ptr);
                    }
                    else{
                        strcat((*th_data).wDir, "/");
                        strcat((*th_data).wDir, ptr);
                    }
                }return "200 working directory changed.\r\n";

            }
            return "500 Syntax error.\r\n";
        case(CDUP_CMD):
            cdup_cmd(th_data);
            return "200 working directory changed. \r\n";
        case (MKD_CMD):
            ptr = strtok_r(NULL, delim, &saveptr);
            result = mkd_cmd(th_data, ptr);
            sprintf(response, "257 \"%s\" created\r\n", ptr);

            if(result ==0){
                return response;
            }
            return "501 Syntax error in parameters or arguments.\r\n";


        case (STOR_CMD):
            (*th_data).fileTransferConn = createFileTransferConn((*th_data).fileTransferAddress,
                                                                 (*th_data).fileTransferPort);
            if((*th_data).fileTransferConn==-1){
                printf("Tak, to tutaj");
                return "425 Can't open data connection.\r\n";
            }

            ptr = strtok_r(NULL, delim, &saveptr);
            int n = stor_cmd(th_data, ptr);
            if(n == 0){
                return "250 Requested file action successful.\r\n";
            }
            return "552 Requested file action aborted.\r\n";
        case DELE_CMD:
            ptr = strtok_r(NULL, delim, &saveptr);
            result = dele_cmd(th_data, ptr);

            if(result!=0){
                return "550 Requested action not taken.\r\n";
            }

            return "250 Requested file action okay, completed.\r\n";

        default:
            printf("Wrong cmd: 500 ->\n");
            return "500 Syntax error, command unrecognized.\r\n";
            break;
    }
}

//funkcja opisującą zachowanie wątku - musi przyjmować argument typu (void *) i zwracać (void *)
void *ThreadBehavior(void *t_data)
{
    // int create_result = 0;
    // pthread_t thread2;

    pthread_detach(pthread_self());
    struct thread_data_t *th_data = (struct thread_data_t*)t_data;

    // create_result = pthread_create(&thread2, NULL, reading, (void *)t_data);
    // if (create_result){
    //    printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
    //    exit(-1);
    // }

    // char buff_write[BUF_SIZE];
    char* buff_write;
    char buff_read[BUF_SIZE];
    while (1){
        int r = read((*th_data).fd, buff_read, BUF_SIZE);
        if(r==0 || (*th_data).doExit==1) break;
        buff_read[r] = '\0';
        printf("%s", buff_read);
        // if((buff_write[0] == '-' && buff_write[1] == '1') || (*th_data).doExit==1) break;
        buff_write = getResponse(buff_read, th_data);
        printf("%s", buff_write);
        // pthread_mutex_lock(&(*th_data).mutex);
        write((*th_data).fd, buff_write, strlen(buff_write));
        pthread_mutex_unlock(&(*th_data).mutex);
    }
    printf("Quitting...\n");
    (*th_data).doExit = 1;

    close((*th_data).fileTransferConn);

    free(th_data);
    num_of_conns--;
    pthread_exit(NULL);
}

//funkcja obsługująca połączenie z nowym klientem
void handleConnection(int connection_socket_descriptor, pthread_mutex_t t_mutex) {
    //wynik funkcji tworzącej wątek
    int create_result = 0;

    //uchwyt na wątek
    pthread_t thread1;

    //dane, które zostaną przekazane do wątku
    //dynamiczne utworzenie instancji struktury thread_data_t o nazwie t_data (+ w odpowiednim miejscu zwolnienie pamięci)
    struct thread_data_t *t_data = malloc(sizeof(struct thread_data_t));
    //wypełnienie pól struktury
    t_data->doExit = 0;
    t_data->fd = connection_socket_descriptor;
    t_data->fileTransferConn = 0;
    t_data->mutex = t_mutex;

    create_result = pthread_create(&thread1, NULL, ThreadBehavior, (void *)t_data);
    if (create_result){
        printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
        exit(-1);
    }
}

int main(int argc, char* argv[])
{
    int server_socket_descriptor;
    int connection_socket_descriptor;
    int bind_result;
    int listen_result;
    char reuse_addr_val = 1;
    struct sockaddr_in server_address;

    //inicjalizacja gniazda serwera

    memset(&server_address, 0, sizeof(struct sockaddr));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(SERVER_PORT);

    inet_pton(AF_INET, "127.0.0.1", &(server_address.sin_addr));

    server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_descriptor < 0)
    {
        fprintf(stderr, "%s: Błąd przy próbie utworzenia gniazda..\n", argv[0]);
        exit(1);
    }
    setsockopt(server_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_addr_val, sizeof(reuse_addr_val));

    bind_result = bind(server_socket_descriptor, (struct sockaddr*)&server_address, sizeof(struct sockaddr));
    if (bind_result < 0)
    {
        fprintf(stderr, "%s: Błąd przy próbie dowiązania adresu IP i numeru portu do gniazda.\n", argv[0]);
        exit(1);
    }

    listen_result = listen(server_socket_descriptor, QUEUE_SIZE);
    if (listen_result < 0) {
        fprintf(stderr, "%s: Błąd przy próbie ustawienia wielkości kolejki.\n", argv[0]);
        exit(1);
    }

    pthread_mutex_t t_mutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_t tempThread;
    pthread_create(&tempThread, NULL, tempClose, NULL);

    while(exitAll!=1)
    {
        connection_socket_descriptor = accept(server_socket_descriptor, NULL, NULL);
        write(connection_socket_descriptor, "220 Service ready for new user.\n", strlen("220 Service ready for new user.\n"));
        if (connection_socket_descriptor < 0)
        {
            fprintf(stderr, "%s: Błąd przy próbie utworzenia gniazda dla połączenia.\n", argv[0]);
            exit(1);
        }
        num_of_conns++;
        handleConnection(connection_socket_descriptor, t_mutex);
    }

    pthread_mutex_destroy(&t_mutex);

    close(server_socket_descriptor);
    return(0);
}
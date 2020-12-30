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

#define SERVER_PORT 1248
#define QUEUE_SIZE 5
#define BUF_SIZE 1000
#define CON_LIMIT 3

#define USER "test"
#define PASS "testhaslo"

// kody komend
#define USER_CMD 1
#define PASS_CMD 2
#define SYST_CMD 3
#define FEAT_CMD 4
#define PWD_CMD 5
#define TYPE_CMD 6
#define PORT_CMD 7
#define LIST_CMD 8
#define UNKNOWN_CMD -1

// typy reprezentacji -- na razie tylko zapisywane, wspierany jest wyłącznie tryb ASCII
#define ASCII_TYPE 1
#define IMAGE_TYPE 2
#define UNSET_TYPE -1

// zmienne globalne potrzebnych danych
// TODO - pozbyć się tych zmiennych (singleton)
int num_of_conns = 0;
int exitAll = 0;
int rep_type = UNSET_TYPE;

//struktura zawierająca dane, które zostaną przekazane do wątku
struct thread_data_t
{
    int fd;
    pthread_mutex_t mutex;
    int doExit;
    int fileTransferConn;
    char* username;
};

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

// funkcja przekształcająca notację portu ftp do numeru portu w postaci integera
int transformPortNumber(char p1[], char p2[]){
    int p1_int = atoi(p1);
    int p2_int = atoi(p2);
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

void *reading(void *t_data)
{
    pthread_detach(pthread_self());
    struct thread_data_t *th_data = (struct thread_data_t*)t_data;

    char buff_read[BUF_SIZE];
    while(1){
        // pthread_mutex_lock(&(*th_data).mutex);
        int r = read((*th_data).fd, buff_read, BUF_SIZE);
        // pthread_mutex_unlock(&(*th_data).mutex);
        if(r==0 || (*th_data).doExit==1) break;
        buff_read[r] = '\0';
        printf("%s", buff_read);
    }
    (*th_data).doExit = 1;

    pthread_exit(NULL);
}

// funkcja odpowiedzialna za wysłanie listy folderu użytkownika; zwraca 0 w przypdku powodzenia i -1 w przypadku porażki
void *sendList(void *t_data) {

    pthread_detach(pthread_self());
    struct thread_data_t *th_data = (struct thread_data_t*)t_data;

    DIR *d;
    struct dirent *dir;
//    char path[100];
//    path[0] = '.';
//    path[1] = '/';
//    for (int i = 0; i < strlen(user); i++) {
//        path[i + 2] = user[i];
//    }
//    path[strlen(user) + 2] = '/';
//    path[strlen(user) + 3] = '\0';
//    d = opendir(path);
    d = opendir("./");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            printf("%s\n", dir->d_name);
            write((*th_data).fileTransferConn, dir->d_name, strlen(dir->d_name));
        }
        closedir(d);
    }
    write((*th_data).fd, "250 Requested file action okay, completed.", strlen("250 Requested file action okay, completed."));
    free(th_data);
    pthread_exit(NULL);
}

// funkcja tworząca wątek odpowiedzialny za przesłanie listy folderu
int handleList(char user[], const int* fileTransferConn, int* fd){

    pthread_t thread;
    struct thread_data_t *t_data = malloc(sizeof(struct thread_data_t));
    t_data->doExit = 0;
    t_data->fd = *fd;
    t_data->username = user;
    t_data->fileTransferConn = *fileTransferConn;
    int create_result = 0;
    create_result = pthread_create(&thread, NULL, sendList, (void *)t_data);
    if (create_result){
        printf("Błąd przy próbie utworzenia wątku przesyłania listy, kod błędu: %d\n", create_result);
        exit(-1);
    }
    return 0;
}

// funkcja rozpoznająca przychodzące komendy ftp i zwracająca odpowidający im kod
int commandCode(char* cmd)
{
    printf("CommandCode generation...\n");
    if(strcmp(cmd, "USER") == 0){
        printf("USER cmd recognized\n");
        return USER_CMD;
    }
    else if(strcmp(cmd, "PASS") == 0){
        printf("PASS cmd recognized\n");
        return PASS_CMD;
    }
    else if(strcmp(cmd, "SYST") == 0){
        printf("SYST cmd recognized\n");
        return SYST_CMD;
    }
    else if(strcmp(cmd, "PWD") == 0){
        printf("PWD cmd recognized\n");
        return PWD_CMD;
    }
    else if(strcmp(cmd, "TYPE") == 0){
        printf("TYPE cmd recognized\n");
        return TYPE_CMD;
    }
    else if(strcmp(cmd, "FEAT") == 0){
        printf("FEAT cmd recognized\n");
        return FEAT_CMD;
    }
    else if(strcmp(cmd, "PORT") == 0){
        printf("PORT cmd recognized\n");
        return PORT_CMD;
    }
    else if(strcmp(cmd, "LIST") == 0){
        printf("LIST cmd recognized\n");
        return LIST_CMD;
    }
    else{
        printf("Unknown cmd\n");
        return UNKNOWN_CMD;
    }
}

// funkcja zwracająca odpoowiedź na daną komendę, a także wywołująca odpowiednie operacje związane z tą komendą
char* getResponse(char* cmd, int* fileTransferConn, int* fd)
{
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
//    char portNum[7];
    int portNum;
    int increment = 0; // zmienna używana przy komendzie TYPE do odpowiedniego przepisania adresu hosta
    int ifSuccess; // zmienna wykorzystywana przy sprawdzaniu powodzenia wywoływanych funkcji
    switch (commandCode(cmdCopy))
    {
        case (USER_CMD):
            ptr = strtok_r(NULL, delim, &saveptr);
            if(strcmp(ptr, USER) == 0){
                printf("USER correct, need pass: 331 ->\n");
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
            break;

            // case (FEAT_CMD):
            //         return "211-Features:\n";
            //     break;

        case (PWD_CMD):
            sprintf(response, "257 /%s/ created\r\n", USER);
            return response;
            break;

        case (TYPE_CMD):
            ptr = strtok_r(NULL, delim, &saveptr);
            if(strcmp(ptr, "A") == 0){
                rep_type = ASCII_TYPE;
                return "200 Command okay.\r\n";
            }
            else if(strcmp(ptr, "I") == 0){
                rep_type = IMAGE_TYPE;
                return "200 Command okay.\r\n";
            }
            return "504 Command not implemented for that parameter.\r\n";
            break;

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
                    increment += (int)strlen(ptr) + 1;
                }
                else{
                    break;
                }
            }
            ptr = strtok_r(NULL, ",", &saveptr);
            if(ptr!=NULL){
                for(int i = 0; i<3; i++){
                    if(*(ptr+i)=='\0') break;
                    p1[i] = *(ptr+i);
                }
                p1[3] = '\0';
                ptr = strtok_r(NULL, delim, &saveptr);
                if(ptr!=NULL){
                    for(int i = 0; i<3; i++){
                        if(*(ptr+i)=='\0') break;
                        p2[i] = *(ptr+i);
                    }
                    p2[3] = '\0';

                    portNum = transformPortNumber(p1, p2);
                    *fileTransferConn = createFileTransferConn(host, portNum);
                    if(*fileTransferConn!=-1){
                        return "200 Command okay.\r\n";
                    }
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
            break;

        case (LIST_CMD):
            if(handleList(USER, fileTransferConn, fd) == 0){
                return "125 Data connection already open; transfer starting.";
            }
            else{
                return "500 Syntax error, command unrecognized.\r\n";
            }

            break;

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
        buff_write = getResponse(buff_read, &(*th_data).fileTransferConn, &(*th_data).fileTransferConn);
        printf("%s", buff_write);
        // pthread_mutex_lock(&(*th_data).mutex);
        write((*th_data).fd, buff_write, strlen(buff_write));
        pthread_mutex_unlock(&(*th_data).mutex);
    }
    (*th_data).doExit = 1;

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

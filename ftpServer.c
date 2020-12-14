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

#define SERVER_PORT 1239
#define QUEUE_SIZE 5
#define BUF_SIZE 1000
#define CON_LIMIT 3

#define USER "test"
#define PASS "testhaslo"

// command codes
#define USER_CMD 1
#define PASS_CMD 2
#define SYST_CMD 3
#define PWD_CMD 4
#define UNKNOWN_CMD -1

int num_of_conns = 0;
int exitAll = 0;

//struktura zawierająca dane, które zostaną przekazane do wątku
struct thread_data_t
{
    int fd;
    pthread_mutex_t mutex;
    int doExit;
};

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
    else{
        printf("Unknown cmd\n");
        return UNKNOWN_CMD;
    }
}

char* getResponse(char* cmd)
{
    char cmdCopy[BUF_SIZE];
    strcpy(cmdCopy, cmd);
    char delim[] = " \r";
    char * saveptr;
    char* ptr = strtok_r(cmdCopy, delim, &saveptr); // 'wycięcie' nazwy komendy
    switch (commandCode(cmdCopy))
    {
    case (USER_CMD):
        ptr = strtok_r(NULL, delim, &saveptr);
        if(strcmp(ptr, USER) == 0){
            printf("USER correct, need pass: 331 ->\n");
            return "331 User name okay, need password.\n";
        }
        else{
            printf("Wrong USER: 530 ->\n");
            return "530 Not logged in.\n";
        }
        break;
    
    case (PASS_CMD):
        ptr = strtok_r(NULL, delim, &saveptr);
        if(strcmp(ptr, PASS) == 0){
            printf("PASS correct. user logged in: 230 ->\n");
            return "230 User logged in, proceed.\n";
        }
        else{
            printf("Wrong PASS: 530 ->\n");
            return "530 Not logged in.\n";
        }
        break;
    
    case (SYST_CMD):       
            return "215 UNIX system type.\n";
        break;

    case (PWD_CMD):
        ptr = strtok_r(NULL, delim, &saveptr);
        if(strcmp(ptr, PASS) == 0){
            printf("PASS correct. user logged in: 230 ->\n");
            return "230 User logged in, proceed.\n";
        }
        else{
            printf("Wrong PASS: 530 ->\n");
            return "530 Not logged in.\n";
        }
        break;

    default:
    printf("Wrong cmd: 500 ->\n");
    return "500 Syntax error, command unrecognized.\n";
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
        buff_write = getResponse(buff_read);
        printf(buff_write);
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
    //TODO dynamiczne utworzenie instancji struktury thread_data_t o nazwie t_data (+ w odpowiednim miejscu zwolnienie pamięci)
    struct thread_data_t *t_data = malloc(sizeof(struct thread_data_t));
    //TODO wypełnienie pól struktury
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

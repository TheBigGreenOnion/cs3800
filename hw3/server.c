/****************************************************************
*   PROGRAM: Assignment 3                                         
*       Robert Higgins
        William Dodds
*                                                                                       
*   Using socket() to create an endpoint for communication. It returns socket descriptor 
*   Using bind() to bind/assign a name to an unnamed socket.                            
*   Using listen() to listen for connections on a socket. Using accept() to accept     
*   a connection on a socket.  It returns the descriptor for the accepted socket      
*                                                                                    
*   gcc -o server server.c -lpthread -D_REENTRANT                                   
*                                                                                  
*****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>  /* define socket */
#include <netinet/in.h>  /* define internet socket */
#include <netdb.h>       /* define internet socket */
#include <pthread.h>
#include <signal.h>

#define SERVER_PORT 9999        /* define a server port number */
#define MAX_CLIENT 10

int client_fds[MAX_CLIENT];  
int ns;
struct sockaddr_in *server_ptr;

int is_running = 1;
pthread_t client_threads[MAX_CLIENT];
char goodbye[512] = "Disconnected by server\0";
char part[512] = "/exit\0";

typedef struct args args;
struct args {
    int ns;
    pthread_mutex_t *m;
    //char *buf[512];
    int (*client_fds)[MAX_CLIENT];
};

args connection_args[MAX_CLIENT];


void write_all(int from, int (*clients)[MAX_CLIENT], char nick[16], char (*msg)[512])
{
    int k = 0;
    for (k=0;k<MAX_CLIENT;k++) {
        if ((*clients)[k] != from && (*clients)[k] > 0) {
            write((*clients)[k], nick, sizeof(nick));
            write((*clients)[k], *msg, sizeof(*msg));
        }
    }
    return;
}

void interrupt_threads(int sig)
{
    int j=0;
    is_running = 0;
    pthread_mutex_lock(connection_args->m); 
    write_all(-1, &client_fds, "SERVER", &goodbye);
    write_all(-1, &client_fds, "SERVER", &part);
    for (j=0;j<MAX_CLIENT;j++) {
        close(client_fds[j]);
    }
    pthread_mutex_unlock(connection_args->m); 

    printf("\nSERVER SHUTTING DOWN IN 10s!\n");


    sleep(10);


    exit(1);
    return;
}

void *connection_thread(void *arg) 
{
    int k;
    int from;
    int (*fds)[MAX_CLIENT];
    char buf[512];
    char nick[16];
    args *connection_args;
    connection_args = arg;

    // read nickname then get rid of any extra garbage
    read(connection_args->ns, buf, sizeof(buf)); 
    from = connection_args->ns;
    fds = connection_args->client_fds[0];
    
    strncpy(nick, buf, 15);
    nick[15] = '\0';
    strcpy(buf, nick);
    strcat(buf, " connected to the server");

    pthread_mutex_lock(connection_args->m);
    write_all(from, fds, "SERVER", &buf); 
    pthread_mutex_unlock(connection_args->m);

    write(from, "SERVER\0", 16);
    strcpy (buf, "Welcome!");
    write(from, buf, sizeof(buf));

    while ((is_running == 1) && (read(connection_args->ns, buf, sizeof(buf)) > 0))
    {
        pthread_mutex_lock(connection_args->m);

        if (strcmp(buf, "/exit") == 0) {
            usleep(100);
            write(from, "SERVER\0", 16);
            strcpy(buf, "Goodbye!\0");
            write(from, buf, sizeof(buf));
            pthread_mutex_unlock(connection_args->m);
            break;
        }

        write_all(from, fds, nick, &buf); 

        pthread_mutex_unlock(connection_args->m);
    }

    pthread_mutex_lock(connection_args->m);

    strcpy(buf, nick);
    strcat(buf, " has disconnected\0");
    close(connection_args->ns);

    strcpy(nick, "SERVER");
    write_all(from, fds, nick, &buf); 
    for (k=0;k<MAX_CLIENT;k++) {
        if ((*fds)[k] == connection_args->ns) {
            (*fds)[k] = -1;
        }
    }

    pthread_mutex_unlock(connection_args->m);

    // on exit
    return;
}

int main()
{
    int sd, k, pid, c, j;
    struct sockaddr_in server_addr = { AF_INET, htons( SERVER_PORT ) };
    server_ptr = &server_addr;
    struct sockaddr_in client_addr = { AF_INET };
    int client_len = sizeof( client_addr );
    int client_id;
    char buf[512], *host;
    char nick[16] = "SERVER";

    signal(SIGINT, (interrupt_threads));

    pthread_mutex_t lock;
    pthread_mutex_init(&lock, NULL);
    
    // Initialize clients for sanity checks later
    for (j=0;j<MAX_CLIENT;j++) {
        client_fds[j] = -1;
    }

    /* create a stream socket */
    if( ( sd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
    {
        perror( "server: socket failed" );
        exit( 1 );
    }
    
    /* bind the socket to an internet port */
    if( bind(sd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1 )
    {
        perror( "server: bind failed" );
        exit( 1 );
    }

   /* listen for clients */
    if( listen( sd, 1 ) == -1 )
    {
        perror( "server: listen failed" );
        exit( 1 );
    }

    while((((ns = accept( sd, (struct sockaddr*)&client_addr, &client_len ))) > 0) ) {

        pthread_mutex_lock(&lock);
        client_id = -1;
        for (j=0;j<MAX_CLIENT;j++) {
            if (client_fds[j] <= 0) {
                //valid
                client_id = j;
                break;
            }
        }

        if (client_id == -1) {
            perror("server: No client slots available");
            strcpy(buf, "Max clients reached on server.\nDisconnecting\0");
            write(ns, buf, sizeof(buf));
            close(ns);
        }

        else {
            // Add client to list and spawn thread
            client_fds[client_id] = ns;
            pthread_mutex_unlock(&lock);

            connection_args[client_id].ns=ns;
            connection_args[client_id].m=&lock;
            connection_args[client_id].client_fds=&client_fds; 

            if (pthread_create(&client_threads[client_id], NULL, &connection_thread, &connection_args[client_id]) != 0) {
                printf("ERROR: COULD NOT CREATE THREAD\n");
            }

        }
        if (is_running == 0) {
            break;
        }
        //usleep(1);
    }

    unlink(server_ptr->sin_addr);
    return(0);
}

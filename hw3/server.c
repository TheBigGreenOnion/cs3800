/****************************************************************
*   PROGRAM: server-template for the assignment                                         
*                                                                                       
*   Using socket() to create an endpoint for communication. It returns socket descriptor 
*   Using bind() to bind/assign a name to an unnamed socket.                            
*   Using listen() to listen for connections on a socket. Using accept() to accept     
*   a connection on a socket.  It returns the descriptor for the accepted socket      
*                                                                                    
*   gcc -o server server.c -lpthread -D_REENTRANT                                   
*                                                                                  
*****************************************************************/
//int counter; mutex m;
/*int main(int argc, char *argv[]);
{
   socket
   bind

   listen(n)  
              

   while(( someint = accept(socket)) > 0)
   {  
      lock(m);
      FD[counter++] = someint;
      unlock(m);
      thr_create(bob, someint, ....)  
   }

   close stuff ...

}

void bob(void* arg)    
{
   get  fd;

   get the host name;
   read in client name;
   print a message about the new client;

   while ((read(FD, buf)) > 0)
   {
      lock(m)
      loop
        write message to each FD
      unlock(m)          
   }
  
   lock(m);
   remove myself from FDarray
   unlock(m) 
   close(FD)
   thr_exit()
}*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>  /* define socket */
#include <netinet/in.h>  /* define internet socket */
#include <netdb.h>       /* define internet socket */
#include <pthread.h>

#define SERVER_PORT 9999        /* define a server port number */
#define MAX_CLIENT 10

int client_fds[MAX_CLIENT];  
pthread_t client_threads[MAX_CLIENT];
int broadcast_filter[MAX_CLIENT];

typedef struct args args;
struct args {
    int ns;
    pthread_mutex_t *m;
    //char *buf[512];
    int (*client_fds)[MAX_CLIENT];
};

args connection_args[MAX_CLIENT];

void *connection_thread(void *arg) 
{
    int k;
    int fd;
    int (*fds)[MAX_CLIENT];
    char buf[512];
    char nick[32];
    args *connection_args;
    connection_args = arg;

    // read nickname
    read(connection_args->ns, nick, sizeof(nick));
    nick[31] = '\0';

    printf("NICK: %s from %d\n", nick, connection_args->ns);
    while (read(connection_args->ns, buf, sizeof(buf)) > 0)
    {
        printf("READ: %x\n", buf);

        pthread_mutex_lock(connection_args->m);

        if (strncmp(buf, "/exit", sizeof(char) * 5) == 0) {
            printf("DERP");
            break;
        }

        fds = connection_args->client_fds[0];
        for (k=0;k<MAX_CLIENT;k++) {
            fd = (*fds)[k];
            printf("FD: %d\n", fd);
            if (fd > 0 && fd != connection_args->ns) {
                //non-fd or own
                write(fd, buf, sizeof(buf)); 
            }
        }
        pthread_mutex_unlock(connection_args->m);
        //usleep(1);
    }

    pthread_mutex_lock(connection_args->m);
    //remove from fdlist

    pthread_mutex_unlock(connection_args->m);

    // on exit
    return;
}

int main()
{
    int sd, ns, k, pid, c, j;
    struct sockaddr_in server_addr = { AF_INET, htons( SERVER_PORT ) };
    struct sockaddr_in client_addr = { AF_INET };
    int client_len = sizeof( client_addr );
    int client_id;
    char buf[512], *host;

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

    while((ns = accept( sd, (struct sockaddr*)&client_addr, &client_len ) ) > 0 ) {
        printf("STILL GOING!");

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


        // Add client to list and spawn thread
        client_fds[client_id] = ns;
        pthread_mutex_unlock(&lock);

        connection_args[client_id].ns=ns;
        connection_args[client_id].m=&lock;
        //connection_args[client_id].buf=&buf;
        connection_args[client_id].client_fds=&client_fds; 

        if (pthread_create(&client_threads[client_id], NULL, &connection_thread, &connection_args[client_id]) != 0) {
            printf("WHAT JUST HAPPENED YO?\n");
        }
        printf("Client %d connected to the server. with fd %d\n", client_id, ns);
        //usleep(1);
    }


    /* data transfer on connected socket ns 
    while( (k = read(ns, buf, sizeof(buf))) != 0)
    {    printf("SERVER(Child) RECEIVED: %s\n", buf);
         write(ns, buf, k);
    }
    close(ns);
    close(sd);

    unlink(server_addr.sin_addr);
*/
    return(0);
}

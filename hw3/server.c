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

#define SERVER_PORT 9999        /* define a server port number */
#define MAX_CLIENT 10

int clients[MAX_CLIENT];  
char global_buf[512];

int main()
{
    int sd, ns, k, pid, c, j;
    struct sockaddr_in server_addr = { AF_INET, htons( SERVER_PORT ) };
    struct sockaddr_in client_addr = { AF_INET };
    int client_len = sizeof( client_addr );
    int client_id;
    char buf[512], *host;
    
    for (j=0;j<MAX_CLIENT;j++) {
        clients[j] = -1;
    }

    /* create a stream socket */
    if( ( sd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
    {
    perror( "server: socket failed" );
    //exit( 1 );
    }
    
    /* bind the socket to an internet port */
    if( bind(sd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1 )
    {
    perror( "server: bind failed" );
    //exit( 1 );
    }

    while(1)
    {

        /* listen for clients */
        if( listen( sd, 1 ) == -1 )
        {
        perror( "server: listen failed" );
        //exit( 1 );
        }

        if( ( ns = accept( sd, (struct sockaddr*)&client_addr,
                           &client_len ) ) == -1 )
        {
            perror( "server: accept failed" );
            //exit( 1 );
        }

        client_id = -1;
        for (j=0;j<MAX_CLIENT;j++) {
            if (clients[j] < 0) {
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
        clients[client_id] = ns;
        //pthread_create() derpderp
        printf("Client %d connected to the server.\n", client_id);
    }


    while(1) {
        sleep(0.1);
    }
    /*

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

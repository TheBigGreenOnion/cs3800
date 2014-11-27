/************************************************************************/ 
/*   PROGRAM NAME: client.c                                             */ 
/*                                                                      */ 
/*   Client creates a socket to connect to Server.                      */ 
/*   When the communication established, Client writes data to server   */ 
/*   and echoes the response from Server.                               */ 
/*                                                                      */ 
/*   To run this program, first compile the server_ex.c and run it      */ 
/*   on a server machine. Then run the client program on another        */ 
/*   machine.                                                           */ 
/*                                                                      */ 
/*   gcc -o client client.c -lpthread -D_REENTRANT                      */
/*   TO RUN:     client  server-machine-name                            */ 
/*                                                                      */ 
/************************************************************************/ 
 
#include <stdio.h> 
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>      
#include <pthread.h>
 
#define SERVER_PORT 9999 

typedef struct args args;
struct args {
    int sd;
};

void handle_int(int sig)
{
    printf("\nUse /exit, /part, or /quit instead of Ctrl-C to quit.\n");
    return;
}

void* reader(void* arg)
{
    args *read_args;
    read_args = arg;

    char read_buf[512];
    int sd = read_args->sd;

    while (read(sd, read_buf, sizeof(read_buf)) > 0)
    {
        printf("%s\n", read_buf);  
        
    }

    return;
}
 
int main( int argc, char* argv[] ) 
{ 
    int sd; 
    int BUFSIZE = 512 * sizeof(char);
    struct sockaddr_in server_addr = { AF_INET, htons( SERVER_PORT ) }; 
    char buf[512];
    char *index;
    struct hostent *hp; 
    pthread_t reader_id;
    struct args reader_args;  

    signal(SIGINT, (handle_int));

    if( argc != 3 ) 
    { 
	printf( "Usage: %s hostname nickname\n", argv[0] ); 
	exit(1); 
    } 
 
    /* get the host */ 
    if( ( hp = gethostbyname( argv[1] ) ) == NULL ) 
    { 
	printf( "%s: %s unknown host\n", argv[0], argv[1] ); 
	exit(1); 
    } 
    bcopy( hp->h_addr_list[0], (char*)&server_addr.sin_addr, hp->h_length ); 

 
    /* create a socket */ 
    if( ( sd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) 
    { 
	perror( "client: socket failed" ); 
	exit(1); 
    } 
 
    /* connect a socket */ 
    if( connect( sd, (struct sockaddr*)&server_addr, 
		 sizeof(server_addr) ) == -1 ) 
    { 
	perror( "client: connect FAILED:" ); 
	exit( 1 ); 
    } 

    /* Upon successful connect, give nickname */ 
    printf("connect() successful! will send a message to server\n"); 
    strcpy(buf, argv[2]);
    write(sd, buf, BUFSIZE);
    printf("Input a string:\n" );

    /* Start new thread for reading */
 
    reader_args.sd = sd;

    if (pthread_create(&reader_id, NULL, (&reader), &reader_args) != 0) {
        printf("Could not create reader thread\n");
        exit(1);
    }

    while( fgets(buf, BUFSIZE, stdin) != NULL) 
    { 
        if ((index = strchr(buf, '\n')) == NULL) {
            buf[0] = '\0';
        } 
        else {
            *index = '\0';
        }

        if (strncmp(buf, "/part", sizeof(char) * 5) == 0 ||
            strncmp(buf, "/quit", sizeof(char) * 5) == 0 || 
            strncmp(buf, "/exit", sizeof(char) * 5) == 0) {

            break;
        }
        else if (buf[0] != '\0') {
            write(sd, buf, BUFSIZE);
        }
    } 

    pthread_cancel(reader_id);
    close(sd); 
    return(0); 
}

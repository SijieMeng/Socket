#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>


int main(int argc, char **argv) {
    int  sockfd;
    ssize_t n;
    char recvline[500];
    struct sockaddr_in servaddr;
    struct hostent *he;

    
    if(argc!=2){
        printf("Usage : gettime <IP address>\n");
        exit(1);
    }
    if ((he = gethostbyname(argv[1])) == NULL) {  // get the host info
        perror("gethostbyname");
        exit(2);
    }
    /* Create a TCP socket */
    if((sockfd=socket(AF_INET,SOCK_STREAM, 0)) < 0){
        perror("socket"); exit(2);}
    
    /* Specify server’s IP address and port */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9311); /* daytime server port */
    
    if (inet_pton(AF_INET, inet_ntoa(*(struct in_addr *)he->h_addr_list[0]), &servaddr.sin_addr) <= 0) {
        perror("inet_pton"); exit(4);
    }
    
    /* Connect to the server */
    if (connect(sockfd,  (struct sockaddr *) &servaddr,sizeof(servaddr))
        < 0 ) {
        perror("connect"); exit(4); }
    
    
    /* Read the date/time from socket */
    while (1) {
        memset(recvline,0,500);
        while ( (n = read(sockfd, recvline, 500)) >= 0) {
            recvline[n] = '\0';        /* null terminate */
            if (n == 0) {// exit
                close(sockfd);
                exit(1);
            }
            printf("%s", recvline);
            /*write command */
            char cmd[256];
            printf("telnet>");
            fgets(cmd,256,stdin);
            if (strcmp(cmd,"exit\n")==0) {// exit
                close(sockfd);
                exit(0);
            }
            write(sockfd, cmd, strlen(cmd));
        }
        
        if (n < 0) { perror("read"); exit(5); }
    }
    
    
}


#include <stdio.h>
#include<netinet/in.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <ctype.h>


#define ENDCHAR(str) (str + strlen(str) - 1) // navigate to the last character

int easeSpaceAtTail(char * input_string) {
    char * endpoint = ENDCHAR(input_string);
    while (isspace(*endpoint)) {
        *endpoint-- = '\0';     //check forward one by one
    }
    return 0;
}
int splitStrToArr(char * input_string, char ** output_array) {
    // if the command is end of '\n', replace it with '\0'
    // otherwise, the execvp will report error
    char *pos;
    if ((pos = strchr(input_string, '\n')) != NULL)
        *pos = '\0';
    
    // get rid of the space at the end of input.
    easeSpaceAtTail(input_string);
    //    printf("%u --%s--\n", strlen(input_string),input_string);

    int idx = 0;
    char * token = strtok(input_string, " ");
    //    printf("--1-- **%s**\n", token);
    while (token != NULL) {
        output_array[idx] = token;
        token = strtok(NULL, " ");
        //        printf("--%d-- **%s**\n", idx, token);
        idx++;
    }
    output_array[idx] = NULL;
    return 0;
}





void runcmd(char *argv[20],char *result){
    
    if (strcmp(argv[0], "pwd") == 0) {
        char buf[500]; // get current dir
        if (getcwd(buf, 500) == NULL) {
            printf("getcwd error\n");
            strcpy(result,"getcwd error\n");
        } else{
            printf("%s\n", buf);
            strcpy(result, buf);
            result[strlen(result)] = '\n';
        }
    } else if (strcmp(argv[0], "cd") == 0) { // change the dir
        if (chdir(argv[1])<0) {
            sprintf(result,"%s: does not exist\n",argv[1]);
        }else{
            result[0]='\1';
        }
    }else if((strcmp(argv[0], "ls") == 0)||(strcmp(argv[0], "mkdir") == 0)||(strcmp(argv[0], "rmdir") == 0)){
        int i=0;
        
        if (argv[1]) {
            char *str = argv[1];
            while (i<strlen(str)) {
                if (str[i] == '-') {
                    strcpy(result,"invalid command!\n");
                    return;
                }
                i++;
            }
        }
        
        int pid;
        int fd[2];
        pipe(fd);

        pid = fork();
        if (pid > 0) {
            //parent
            while(waitpid(pid, NULL, 0)!=pid);
            close(fd[1]);
            read(fd[0], result, 500);
            dup2(fd[0], 0);
        }
        else if (pid == 0) { // child
            dup2(fd[1], 2);
            dup2(fd[1], 1); // redirect stdout
            
            int err = execvp(argv[0], argv);
            exit(err);
        } else {
            printf("fork error\n");
            exit(EXIT_FAILURE);
        }
    }else{
        strcpy(result,"invalid command!\n");
    }
}




int main(int argc, char **argv) {
    int   listenfd, connfd;
    socklen_t clilen;
    struct sockaddr_in servaddr, cliaddr;
    char buff[100];
    time_t ticks;
    
    /* Create a TCP socket */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    
    /* Initialize server's address and well-known port */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(9311);   /* daytime server */
    
    /* Bind server’s address and port to the socket */
    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    /* Convert socket to a listening socket – max 100 pending clients*/
    listen(listenfd, 100);
    
    
    while ( 1 ) {
        /* Wait for client connections and accept them */
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        
        /* Retrieve system time */
        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        printf("%s\r\n", ctime(&ticks));
        
        /* Write to socket */
        write(connfd, buff, strlen(buff));
        
        //reading  command
        ssize_t read_n;
        char cmd[256];
        while ( (read_n = read(connfd, cmd, 256)) >= 0) {
            
            if (read_n == 0 ) {// exit
                perror("connected failed");
                close(connfd);
                exit(1);
            }else{
                cmd[read_n] = '\0';
                printf("%s", cmd);
                
                char *argv[20];
                splitStrToArr(cmd, argv);  //argv[0]='ls'

                
                char result[500]={'\0'};
                runcmd(argv, result);
                /*write result to client*/
                if (result[0] == '\0') {
                    result[0] = '\1';
                }
                write(connfd, result, strlen(result));
            }
        }
        if (read_n < 0) { perror("read"); close(connfd);exit(5); }
        
        
        
       
        
       
    }
}


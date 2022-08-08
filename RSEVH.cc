

#include "FDSH.h"
#include "TIAS.h"

extern char **Environm;


void run_it (int sd, const char* cmd1, char* const Args33[], const char* Filout) {
    char answerW[MAX_LEN];
    int status;
    char messagesZ[MAX_LEN];  

    int Childui = fork();

    if (Childui == 0) { 
       
        close(1);
        close(2);
        
        int ofd = open(Filout, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, S_IRUSR | S_IWUSR);
        if (ofd < 0) {
            snprintf(answerW, MAX_LEN, "FAIL %d output redirection error: %s\r\n", errno, strerror(errno));
            send(sd, answerW, strlen(answerW), 0);
            snprintf(messagesZ, MAX_LEN, "%s: output redirection error %d: %s\n", 
                     __FILE__, errno, strerror(errno));
            logger(messagesZ);
            if (debugs[DEBUG_COMM]) {
                snprintf(messagesZ, MAX_LEN, "%s: <-- %s\n", __FILE__, answerW);
                logger(messagesZ);
            } /* DEBUG_COMM */
            exit(err_exec);
        }
        while (dup2(ofd,2) < 0 && (errno == EBUSY || errno == EINTR))
            /* NOP */ ;

       
        execvp(cmd1, Args33);

       
        snprintf(answerW, MAX_LEN, "FAIL %d exec error: %s\r\n", errno, strerror(errno));
        send(sd, answerW, strlen(answerW), 0);
        snprintf(messagesZ, MAX_LEN, "%s: exec error %d: %s\n", __FILE__, errno, strerror(errno));
        logger(messagesZ);
        if (debugs[DEBUG_COMM]) {
            snprintf(messagesZ, MAX_LEN, "%s: <-- %s\n", __FILE__, answerW);
            logger(messagesZ);
        } 
        close(1);
        close(2);
        exit(err_exec);
    }

    else { 
        waitpid(Childui, &status,0);
        int r = WEXITSTATUS(status);
        snprintf(messagesZ, MAX_LEN, "%s: command completed with status %d\n", __FILE__, r);
        logger(messagesZ);
        if (r != err_exec) {  
            if (r != 0) {
                snprintf(answerW, MAX_LEN, "ERR %d command completed with a non-null exit code\r\n", r);
            }
            else {
                snprintf(answerW, MAX_LEN, "OK 0 command completed\r\n");
            }
            send(sd, answerW, strlen(answerW), 0);
            if (debugs[DEBUG_COMM]) {
                snprintf(messagesZ, MAX_LEN, "%s: <-- %s\n", __FILE__, answerW);
                logger(messagesZ);
            } 
        }
    }
}



void* shell_rec(client_t* clnt){
    int error =0,ls =0;
    socklen_t len = sizeof(error);
    printf("started function:");
    
    while(1){
       
        ls = getsockopt(clnt -> Fedr, SOL_SOCKET, SO_KEEPALIVE ,&error, &len);
        printf("getsockopt : %d with error :%s\n",ls,strerror(error));
        if(ls == -1){
            clnt -> Is_busy = false;
            break;
        }
        sleep(3);

    }
    return 0;
}



void* shell_client (client_t* clnt) {
    int sd = clnt -> Fedr;
  
    char* ip = clnt -> ip;
    clnt->Is_busy = true;
  

   
    char command[MAX_LEN];  
    command[MAX_LEN - 1] = '\0';
    char* com_tok[MAX_LEN]; 
    size_t num_tok;         
    bool no_command = true; 
    char ans[MAX_LEN];
    int n;
    char msg[MAX_LEN];  

    snprintf(msg, MAX_LEN, "%s: new client from %s assigned socket descriptor %d\n",
             __FILE__, ip, sd);
    logger(msg);
    snprintf(msg, MAX_LEN, 
             "Welcome to shfd v.1 [%s]. CPRINT and shell comands spoken here.\r\n",
             ip);
    send(sd, msg, strlen(msg),0);

   
    char out_file[MAX_LEN];
    snprintf(out_file, MAX_LEN, "/tmp/shfd-tmp-%d", sd);

   

    while ((n = readline(sd, command, MAX_LEN-1)) != recv_nodata) {
        

        if ( n >= 1 && command[n-1] == '\r' ) 
            command[n-1] = '\0';
      
        num_tok = str_tokenize(command, com_tok, strlen(command));
        com_tok[num_tok] = 0;      // null termination for execv*
        if (debugs[DEBUG_COMM]) {
            snprintf(msg, MAX_LEN, "%s: --> %s\n", __FILE__, command);
            logger(msg);
        } 

        if (strlen(command) == 0) {
            snprintf(ans, MAX_LEN, "FAIL %d provide a command to run\r\n", EBADMSG);
            send(sd, ans, strlen(ans), 0);
            if (debugs[DEBUG_COMM]) {
                snprintf(msg, MAX_LEN, "%s: <-- %s\n", __FILE__, ans);
                logger(msg);
            } 
        }

        else if (strncmp(command, "CPRINT", strlen("CPRINT")) == 0) {  // print last output
            if (no_command) {
                snprintf(ans, MAX_LEN, "FAIL %d no command executed in this session\r\n", EIO);
                send(sd, ans, strlen(ans), 0);
                if (debugs[DEBUG_COMM]) {
                    snprintf(msg, MAX_LEN, "%s: <-- %s\n", __FILE__, ans);
                    logger(msg);
                } 
            }
            else {
                int r;
                int ofd = open(out_file, O_RDONLY);
                if (ofd < 0) {
                    snprintf(ans, MAX_LEN, "ERR %d retrieving last output: %s\r\n", 
                             errno, strerror(errno));
                    send(sd, ans, strlen(ans), 0);
                    if (debugs[DEBUG_COMM]) {
                        snprintf(msg, MAX_LEN, "%s: <-- %s\n", __FILE__, ans);
                        logger(msg);
                    } 
                    snprintf(msg, MAX_LEN, "%s: %s while retrieving last output\n", 
                             __FILE__, strerror(errno));
                    logger(msg);
                }
                memset(ans, 0, MAX_LEN);
                while ((r = read(ofd, ans, MAX_LEN)) != 0) {
                    if (r < 0) {
                        snprintf(ans, MAX_LEN, "FAIL %d %s\r\n", errno, strerror(errno));
                        send(sd, ans, strlen(ans), 0);
                        if (debugs[DEBUG_COMM]) {
                            snprintf(msg, MAX_LEN, "%s: <-- %s\n", __FILE__, ans);
                            logger(msg);
                        } 
                        break;
                    }
                    send(sd, ans, strlen(ans), 0);
                    memset(ans, 0, MAX_LEN);
                }
                snprintf(ans, MAX_LEN, "OK 0 end of output\r\n");
                send(sd, ans, strlen(ans), 0);
                if (debugs[DEBUG_COMM]) {
                    snprintf(msg, MAX_LEN, "%s: <-- %s\n", __FILE__, ans);
                    logger(msg);
                } 
                close(ofd);
            }
        }
        else { 
            unlink(out_file);
            no_command = false;
            run_it(sd, com_tok[0], com_tok, out_file);
        }

    }

   
    printf("Done with connection\n");
    snprintf(msg, MAX_LEN, "%s: Client on socket Descriptor %d left, closing\n", __FILE__, sd);
    logger(msg);
    shutdown(sd, SHUT_RDWR);
    close(sd);
    clnt->Is_busy = false;
    clnt->Fedr = -1;
    unlink(out_file);
   
    return 0;
}

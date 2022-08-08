
#include "FDSH.h"


rwexcl_t** Frkos1;
size_t Frkos_Size;

bool* OpnedFiles;


replica_server* Tfgr;



void update_replica_servers(replica_server* rs){
   
    Tfgr = rs;
    char messages[MAX_LEN]; 
    snprintf(messages,MAX_LEN," update_replica_servers called\n");
    logger(messages); 
    
}


void sync_with_server(int fd){
    char msg[MAX_LEN];  // logger string
    snprintf(msg, MAX_LEN, "sending sync fd %d\n",fd);
    logger(msg);
    int y=0;
    while(Tfgr[y].ip!=NULL){
        
        snprintf(msg,MAX_LEN,"Sent sync request to %s:%d  \n",Tfgr[y].ip,Tfgr[y].port);
        logger(msg);
        y++;

        
           
    
    }

    
}



int file_init (const char* filename) {
    char msg[MAX_LEN];  // logger string
    snprintf(msg, MAX_LEN, "%s: attempting to open %s\n", __FILE__, filename);
    logger(msg);

   
    int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if ( fd == -1 ) {
        snprintf(msg, MAX_LEN, "%s: open error: %s\n", __FILE__, strerror(errno));
        logger(msg);
        return -1;
    }

  
    rwexcl_t* lck = new rwexcl_t;
    pthread_mutex_init(&lck -> mutex, 0);
    pthread_cond_init(&lck -> can_write,0);
    lck -> reads = 0;
    lck -> fd = fd;
    lck -> owners = 1;
    lck -> name = new char[strlen(filename) + 1];
    strcpy(lck -> name, filename);

    Frkos1[fd] = lck;
    snprintf(msg, MAX_LEN, "%s: %s opened on descriptor %d\n", __FILE__, filename, fd);
    logger(msg);
    return fd;
}


int file_exit (int fd) {
    char msg[MAX_LEN];  // logger string
    snprintf(msg, MAX_LEN, "%s: attempting to close descriptor %d\n", __FILE__, fd);
    logger(msg);

    if (Frkos1[fd] == 0)
        return err_nofile;


    pthread_mutex_lock(&Frkos1[fd] -> mutex);
    while (Frkos1[fd] -> reads != 0) {

        pthread_cond_wait(&Frkos1[fd] -> can_write, &Frkos1[fd] -> mutex);
    }
 

    Frkos1[fd] -> owners --;
    if ( Frkos1[fd] -> owners != 0) {
        snprintf(msg, MAX_LEN, "%s: descriptor %d owned by other clients\n", __FILE__, fd);
        logger(msg);
 
        pthread_cond_broadcast(&Frkos1[fd] -> can_write);
        pthread_mutex_unlock(&Frkos1[fd] -> mutex);
        return 0;
    }


    int closed = close(Frkos1[fd] -> fd);
    delete[] Frkos1[fd] -> name;
    delete Frkos1[fd];
    Frkos1[fd] = 0;
    snprintf(msg, MAX_LEN, "%s: descriptor %d closed\n", __FILE__, fd);
    logger(msg);


    return closed;
}


int write_excl(int fd, const char* stuff, size_t stuff_length) {
    int result;
    char msg[MAX_LEN];  // logger string

    if (Frkos1[fd] == 0)
        return err_nofile;

   
    pthread_mutex_lock(&Frkos1[fd] -> mutex);
   
    while (Frkos1[fd] -> reads != 0) {
       
        pthread_cond_wait(&Frkos1[fd] -> can_write, &Frkos1[fd] -> mutex);
    }
   

    if (debugs[DEBUG_DELAY]) {
       
        snprintf(msg, MAX_LEN, "%s: debug write delay 5 seconds begins\n", __FILE__);
        logger(msg);
        sleep(5);
        snprintf(msg, MAX_LEN, "%s: debug write delay 5 seconds ends\n", __FILE__);
        logger(msg);
       
    } 

    result = write(fd,stuff,stuff_length);
    if (result == -1) {
        snprintf(msg, MAX_LEN, "%s: write error: %s\n", __FILE__, strerror(errno));
        logger(msg);
    }
    
  
    pthread_cond_broadcast(&Frkos1[fd] -> can_write);
   
    pthread_mutex_unlock(&Frkos1[fd] -> mutex);
    return result;  
}


int seek_excl(int fd, off_t offset) {
    int result;
    char msg[MAX_LEN]; 

    if (Frkos1[fd] == 0)
        return err_nofile;

    if (debugs[DEBUG_FILE]) {
        snprintf(msg, MAX_LEN, "%s: seek to %d into descriptor %d\n", __FILE__, (int)offset, fd);
        logger(msg);
    }

   
    pthread_mutex_lock(&Frkos1[fd] -> mutex);
   
    while (Frkos1[fd] -> reads != 0) {
       
        pthread_cond_wait(&Frkos1[fd] -> can_write, &Frkos1[fd] -> mutex);
    }
   

    result = lseek(fd, offset, SEEK_CUR);
    if (result == -1) {
        snprintf(msg, MAX_LEN, "%s: lseek error: %s\n", __FILE__, strerror(errno));
        logger(msg);
    }
    
    
    pthread_cond_broadcast(&Frkos1[fd] -> can_write);
   
    pthread_mutex_unlock(&Frkos1[fd] -> mutex);
    return result; 
}


int read_excl(int fd, char* stuff, size_t stuff_length) {
    int result;
    char msg[MAX_LEN]; 

    if (Frkos1[fd] == 0)
        return err_nofile;    

    if (debugs[DEBUG_FILE]) {
        snprintf(msg, MAX_LEN, "%s: read %lu bytes from descriptor %d\n", __FILE__, stuff_length, fd);
        logger(msg);
    }

    pthread_mutex_lock(&Frkos1[fd] -> mutex);
   
    Frkos1[fd] -> reads ++;
    pthread_mutex_unlock(&Frkos1[fd] -> mutex);

  

    if (debugs[DEBUG_DELAY]) {
        
        snprintf(msg, MAX_LEN, "%s: debug read delay 20 seconds begins\n", __FILE__);
        logger(msg);
        sleep(20);
        snprintf(msg, MAX_LEN, "%s: debug read delay 20 seconds ends\n", __FILE__);
        logger(msg);
      
    } 

    result = read(fd, stuff, stuff_length);

  
    pthread_mutex_lock(&Frkos1[fd] -> mutex);
    Frkos1[fd] -> reads --;
 
    if (Frkos1[fd] -> reads == 0)
        pthread_cond_broadcast(&Frkos1[fd] -> can_write);
    pthread_mutex_unlock(&Frkos1[fd] -> mutex);

    return result;
}



void* replica_sync_request_recieve(int rssock){
    char msg[MAX_LEN];
    char req[MAX_LEN];
    if(OpnedFiles==NULL){
        OpnedFiles = new bool[Frkos_Size];
        for (size_t i = 0; i < Frkos_Size; i++)
            OpnedFiles[i] = false;
    }

    struct sockaddr_in client_addr; 
    socklen_t client_addr_len = sizeof(client_addr);

    snprintf(msg, MAX_LEN, "Replica server started %d \n",rssock);
    logger(msg);

    int ssock,n;
    
    while (1) {
       
        ssock = accept(rssock, (struct sockaddr*)&client_addr, &client_addr_len);
        if (ssock < 0) {
            if (errno == EINTR) continue;
            snprintf(msg, MAX_LEN, "%s: replica server accept: %s\n", __FILE__, strerror(errno));
            logger(msg);
            snprintf(msg, MAX_LEN, "%s: the replica server died.\n", __FILE__);
            logger(msg);
            return 0;
        }

        n= read(ssock,req,MAX_LEN-1);
        if(n<0){
            perror("Error Reading from server");
            exit(1);
        }

        char *ptr = strtok(req," ");
        if(ptr!=NULL){
            if(strcmp(ptr,"OPENED") == 0){
                ptr = strtok(NULL," ");
                if(ptr!=NULL){
                    int fd = atoi(ptr);
                    if (fd < 0)
                        snprintf(msg, MAX_LEN, "FAIL %d %s", errno, strerror(errno));
                    else {
                        if(OpnedFiles[fd] == true){
                            snprintf(msg, MAX_LEN, "OK %d file already opened", fd);
                        }else{
                            snprintf(msg, MAX_LEN, "OK %d file opened", fd);
                            OpnedFiles[fd] = true;

                        }
                        
                    }
                n = write(ssock,msg,MAX_LEN);
                if(n<0){
                    perror("Error writing to socket");
                    exit(1);
                }
                }
            }else{
                n = write(ssock,"ERR 5 Invalid command recieved",MAX_LEN);
                if(n<0){
                    perror("Error writing to socket");
                    exit(1);
                }
            }
        }


        snprintf(msg, MAX_LEN, "Replica server recived %s \n",req);
        logger(msg);
        
    }

    return 0;
}









void* file_client (client_t* clnt) {
    
    char* ip = clnt -> ip;

    char req[MAX_LEN]; 
    char msg[MAX_LEN]; 
    int n;
    if(OpnedFiles==NULL){
        OpnedFiles = new bool[Frkos_Size];
        for (size_t i = 0; i < Frkos_Size; i++)
            OpnedFiles[i] = false;
    }

   
    req[MAX_LEN-1] = '\0';

    snprintf(msg, MAX_LEN, "in thread %d \n",clnt->Osts);
    logger(msg);

    pthread_mutex_lock(&(clnt->lock));


    pthread_cond_wait(&(clnt -> thread_cond),&(clnt->lock));

    int sd = clnt -> Fedr;

    snprintf(msg, MAX_LEN, "%s: new client from %s assigned socket descriptor %d\n",
             __FILE__, ip, sd);
    logger(msg);
    snprintf(msg, MAX_LEN, 
             "Welcome to shfd v.1 [%s]. FOPEN FSEEK FREAD FWRITE FCLOSE QUIT spoken here.\r\n",
             ip);
    send(sd, msg, strlen(msg),0);

    
    while ((n = readline(sd,req,MAX_LEN-1)) != recv_nodata) {
        char* ans = new char[MAX_LEN];
      
        ans[MAX_LEN-1] = '\0';

        if ( n > 1 && req[n-1] == '\r' ) 
            req[n-1] = '\0';
        if (debugs[DEBUG_COMM]) {
            snprintf(msg, MAX_LEN, "%s: --> %s\n", __FILE__, req);
            logger(msg);
        } 
        if ( strncasecmp(req,"QUIT",strlen("QUIT")) == 0 ) { 
            snprintf(msg, MAX_LEN, "%s: QUIT received from client %d (%s), closing\n", __FILE__, sd, ip);
            logger(msg);
            if (debugs[DEBUG_COMM]) {
                snprintf(msg, MAX_LEN, "%s: <-- OK 0 nice talking to you\n", __FILE__);
                logger(msg);
            } 
            send(sd,"OK 0 nice talking to you\r\n", strlen("OK 0 nice talking to you\r\n"),0);
            shutdown(sd, SHUT_RDWR);
            close(sd);
            delete[] ans;
            snprintf(msg, MAX_LEN, "%s: Attempting to close the files opened by this client\n", __FILE__);
            logger(msg);
            for (size_t i = 0; i < Frkos_Size; i++)
                if (OpnedFiles[i])
                    file_exit(i);
            delete[] OpnedFiles;
            delete clnt;
            return 0;
        }
      


        else if (strncasecmp(req,"FOPEN",strlen("FOPEN")) == 0 ) {
            int idx = next_arg(req,' ');
            if (idx == -1 ) {
                snprintf(ans,MAX_LEN,"FAIL %d FOPEN requires a file name", EBADMSG);
            }
            else { 
                char filename[MAX_LEN];
               
                if (req[idx] == '/') {
                    snprintf(filename, MAX_LEN, "%s", &req[idx]);
                }
                else { 
                    char cwd[MAX_LEN];
                    getcwd(cwd, MAX_LEN);
                    snprintf(filename, MAX_LEN, "%s/%s", cwd, &req[idx]);
                }

               
                int fd = -1;
                for (size_t i = 0; i < Frkos_Size; i++) {
                    if (Frkos1[i] != 0 && strcmp(filename, Frkos1[i] -> name) == 0) {
                        fd = i;
                        pthread_mutex_lock(&Frkos1[fd] -> mutex);
                        if (! OpnedFiles[fd])  
                            Frkos1[fd] -> owners ++;
                        pthread_mutex_unlock(&Frkos1[fd] -> mutex);
                        OpnedFiles[fd] = true;
                        break;
                    }
                }
                if (fd >= 0) { 
                    snprintf(ans, MAX_LEN,
                             "ERR %d file already opened, please use the supplied identifier", fd);
                    sync_with_server(fd);
                }
                else { 
                    fd = file_init(filename);
                    if (fd < 0)
                        snprintf(ans, MAX_LEN, "FAIL %d %s", errno, strerror(errno));
                    else {
                        snprintf(ans, MAX_LEN, "OK %d file opened, please use supplied identifier", fd);
                        OpnedFiles[fd] = true;
                        sync_with_server(fd);
                    }
                }
                
            }
        } 

       
        else if (strncasecmp(req,"FREAD",strlen("FREAD")) == 0 ) {
            int idx = next_arg(req,' ');
            if (idx == -1) 
                snprintf(ans,MAX_LEN,"FAIL %d FREAD requires a file identifier", EBADMSG);
            else {
                int idx1 = next_arg(&req[idx],' ');
                if (idx1 == -1) 
                    snprintf(ans,MAX_LEN,"FAIL %d FREAD requires a number of bytes to read", EBADMSG);
                else {
                    idx1 = idx + idx1;
                    req[idx1 - 1] = '\0';
                    if (debugs[DEBUG_COMM]) {
                        snprintf(msg, MAX_LEN, "%s: (before decoding) will read %s bytes from %s \n",
                                 __FILE__, &req[idx1], &req[idx]); 
                        logger(msg);
                    }
                    idx = atoi(&req[idx]);  
                    idx1 = atoi(&req[idx1]);
                    if (debugs[DEBUG_COMM]) {
                        snprintf(msg, MAX_LEN, "%s: (after decoding) will read %d bytes from %d \n",
                                 __FILE__, idx1, idx); 
                        logger(msg);
                    }
                    if (idx <= 0 || idx1 <= 0)
                        snprintf(ans, MAX_LEN,
                                 "FAIL %d identifier and length must both be positive numbers", EBADMSG);
                    else { 
                        
                        char* read_buff = new char[idx1+1];
                        int result = read_excl(idx, read_buff, idx1);
                        
                        if (result == err_nofile) {
                            snprintf(ans, MAX_LEN, "FAIL %d bad file descriptor %d", EBADF, idx);
                        }
                        else if (result < 0) {
                            snprintf(ans, MAX_LEN, "FAIL %d %s", errno, strerror(errno));
                        }
                        else {
                            read_buff[result] = '\0';
                           
                            delete[] ans;
                            ans = new char[40 + result];
                            snprintf(ans, MAX_LEN, "OK %d %s", result, read_buff);
                        }
                        delete [] read_buff;
                    }
                }
            }
        }

       
        else if (strncasecmp(req,"FWRITE",strlen("FWRITE")) == 0 ) {
            int idx = next_arg(req,' ');
            if (idx == -1) 
                snprintf(ans,MAX_LEN,"ERROR %d FWRITE required a file identifier", EBADMSG);
            else {
                int idx1 = next_arg(&req[idx],' ');
                if (idx1 == -1) 
                    snprintf(ans,MAX_LEN,"FAIL %d FWRITE requires data to be written", EBADMSG);
                else {
                    idx1 = idx1 + idx;
                    req[idx1 - 1] = '\0';
                    idx = atoi(&req[idx]);  
                    if (idx <= 0)
                        snprintf(ans,MAX_LEN,
                                 "FAIL %d identifier must be positive", EBADMSG);
                    else { 
                        if (debugs[DEBUG_FILE]) {
                            snprintf(msg, MAX_LEN, "%s: will write %s\n", __FILE__, &req[idx1]);
                            logger(msg);
                        }
                        int result = write_excl(idx, &req[idx1], strlen(&req[idx1]));
                        if (result == err_nofile)
                            snprintf(ans, MAX_LEN, "FAIL %d bad file descriptor %d", EBADF, idx);
                        else if (result < 0) {
                            snprintf(ans, MAX_LEN, "FAIL %d %s", errno, strerror(errno));
                        }
                        else {
                            snprintf(ans, MAX_LEN, "OK 0 wrote %d bytes", result);
                        }
                    }
                }
            }
        } 

       
        else if (strncasecmp(req,"FSEEK",strlen("FSEEK")) == 0 ) {  
            int idx = next_arg(req,' ');
            if (idx == -1) 
                snprintf(ans,MAX_LEN,"FAIL %d FSEEK requires a file identifier", EBADMSG);
            else {
                int idx1 = next_arg(&req[idx],' ');
                if (idx1 == -1) 
                    snprintf(ans,MAX_LEN,"FAIL %d FSEEK requires an offset", EBADMSG);
                else {
                    idx1 = idx1 + idx;
                    req[idx1 - 1] = '\0';
                    idx = atoi(&req[idx]); 
                    idx1 = atoi(&req[idx1]);
                    if (idx <= 0)
                        snprintf(ans,MAX_LEN,
                                 "FAIL %d identifier must be positive", EBADMSG);
                    else { 
                        int result = seek_excl(idx, idx1);
                        if (result == err_nofile)
                            snprintf(ans, MAX_LEN, "FAIL %d bad file descriptor %d", EBADF, idx);
                        else if (result < 0) {
                            snprintf(ans, MAX_LEN, "FAIL %d %s", errno, strerror(errno));
                        }
                        else {
                            snprintf(ans, MAX_LEN, "OK 0 offset is now %d", result);
                        }
                    }
                }
            } 
        } 

        
        else if (strncasecmp(req,"FCLOSE",strlen("FCLOSE")) == 0 ) {  
            int idx = next_arg(req,' ');
            if (idx == -1) 
                snprintf(ans,MAX_LEN,"FAIL %d FCLOSE requires a file identifier", EBADMSG);
            else {
                idx = atoi(&req[idx]); 
                if (idx <= 0)
                    snprintf(ans,MAX_LEN,
                             "FAIL %d identifier must be positive", EBADMSG);
                else { 
                    int result = file_exit(idx);
                    OpnedFiles[idx] = false;
                    if (result == err_nofile)
                        snprintf(ans, MAX_LEN, "FAIL %d bad file descriptor %d", EBADF, idx);
                    else if (result < 0) {
                        snprintf(ans, MAX_LEN, "FAIL %d %s", errno, strerror(errno));
                    }
                    else {
                        snprintf(ans, MAX_LEN, "OK 0 file closed");
                    }
                }
            }
        } 

       
        else {
            int idx = next_arg(req,' ');
            if ( idx == 0 )
                idx = next_arg(req,' ');
            if (idx != -1)
                req[idx-1] = '\0';
            snprintf(ans,MAX_LEN,"FAIL %d %s not understood", EBADMSG, req);
        }

       

      
        if (debugs[DEBUG_COMM]) {
            snprintf(msg, MAX_LEN, "%s: <-- %s\n", __FILE__, ans);
            logger(msg);
        } 
        send(sd,ans,strlen(ans),0);
        send(sd,"\r\n",2,0);        
        delete[] ans;

        if(clnt -> Offdwn ==1){
            close(sd);
            for (size_t i = 0; i < Frkos_Size; i++)
            if (OpnedFiles[i])
                file_exit(i);
           
            exit(0);
        }


    } 
    pthread_mutex_unlock(&(clnt->lock));

   
    snprintf(msg, MAX_LEN, "%s: client on socket descriptor %d (%s) went away, closing\n",
             __FILE__, sd, ip);
    logger(msg);
    shutdown(sd, SHUT_RDWR);
    close(sd);
    clnt->Osts = recreate;  
    for (size_t i = 0; i < Frkos_Size; i++)
        if (OpnedFiles[i])
            file_exit(i);
   
    pthread_cancel(pthread_self());
    
    return 0;
}

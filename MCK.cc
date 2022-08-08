
#include "FDSH.h"
#include "FILES1.h"

#include <stdio.h>      
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <vector>
#include <string>


char LogFiles[20] = "bbserv.log";
char PIDfiles[20] = "bbserv.pid";
char Confuguration[100] = "bbserve.conf";
char FilesBB[100] = "bbfile";
long int Shok1, fsok1, Rsok1, Sdk1, Bdk1;          // master_sockets of all

bool Duuyuh;
sync_thread* Snk_thr_resr;
pthread_t* rcr1;
pthread_mutex_t Read_luck;
pthread_mutex_t Write_lock;
int number_read;
int hgf1 = 0;

bool alive123;

pthread_mutex_t logger_mutex;

bulletin_resources bulletin_res;
sync_master syns;
FILES1* clients;
pthread_t* bulletin_clients;
client_t* shell_clnt;
threadpool_t *t_pool;
int t_incr = 0;
int t_max = 0;
struct replica_server* r_servers;
int r_servers_count = 0;
char progname[100];
extern char **Environm;

int debug_opt;


bool debugs[3] = {false, false, false};

void logger(const char * msg) {
    pthread_mutex_lock(&logger_mutex);
    time_t tt = time(0);
    char* ts = ctime(&tt);
    ts[strlen(ts) - 1] = '\0';
    printf("%s: %s", ts, msg);
    fflush(stdout);
    pthread_mutex_unlock(&logger_mutex);
}


void sigquit_handler(int s){
    char command_end[1024];
    sprintf(command_end, "kill -9 %d", hgf1);
    if (hgf1 > 0)
        popen(command_end, "r");
    sprintf(command_end, "kill -9 %d", getpid());
    popen(command_end, "r");
}

void sigup_handler(int s){
   
    FILE* pfile;
    char buffer[100];
    char option[100];
    
    for (int i = 0; i < syns.get_peers_num(); i++)
        syns.close_connection(i);
    
    while(syns.get_peers_num())
        syns.erase_peer(0);
    
    pfile = fopen(Confuguration, "rw+");
    while(fscanf(pfile, "%[^=]%*[=]", option) != EOF){
        int result;
      
        if ( (strcmp(option, "THMAX") == 0) ){
            fscanf(pfile, "%d", &result);
         
        }else if ( (strcmp(option, "BBPORT") == 0) ){
            fscanf(pfile, "%d", &result);
          
        }else if ( (strcmp(option, "BBFILE") == 0) ){
            result = fscanf(pfile, "%[^\n]", FilesBB);
           
        }else if ( (strcmp(option, "SYNCPORT") == 0) ){
            fscanf(pfile, "%d", &result);
          
        }else if ( (strcmp(option, "DAEMON") == 0) ){
            char var[10];
            result = fscanf(pfile, "%[^\n]", var);
            if ( (!strcmp(var, "true")) || (!strcmp(var, "1")) ){
              
            }
            if ( (!strcmp(var, "false")) || (!strcmp(var, "0")) ){
             
            }
          
        }else if ( (strcmp(option, "DEBUG") == 0) ){
            char var[10];
            result = fscanf(pfile, "%[^\n]", var);
            
            if ( (!strcmp(var, "true")) || (!strcmp(var, "1")) ){
                debug_opt = 1;
            }
            if ( (!strcmp(var, "false")) || (!strcmp(var, "0")) ){
                debug_opt = 0;
            }
           
        }else if ( (strcmp(option, "PEERS") == 0)){
            char var[20];
            int port;
            while( result = fscanf(pfile, "%[^:\n]%*[:]%d%*[ ]", var, &port) ){
              
                if (!strcmp(var, "localhost"))
                    sprintf(var, "127.0.0.1");
                syns.push_peer(var, port);
            }
        }
        fscanf(pfile, "%*[\n]");

    }
    
    
    fclose(pfile);
    
    syns.init();
    for (int i = 0; (i < syns.get_peers_num()) ;i++){
        syns.connectto(i);
    }

}



void ip_to_dotted(unsigned int ip, char* buffer) {
    char* ipc = (char*)(&ip);
    sprintf(buffer, "%d.%d.%d.%d", ipc[0], ipc[1], ipc[2], ipc[3]);
}

int next_arg(const char* line, char delim) {
    int arg_index = 0;
    char msg[MAX_LEN];  

    
    while ( line[arg_index] != '\0' && line[arg_index] != delim)
        arg_index++;
    
    if (line[arg_index] == '\0') {
        if (debugs[DEBUG_COMM]) {
            snprintf(msg, MAX_LEN, "%s: next_arg(%s, %c): no argument\n", __FILE__, line ,delim);
            logger(msg);
        } 
        return -1;
    }
   
    arg_index++;
   
    if (line[arg_index] == '\0') {
        if (debugs[DEBUG_COMM]) {
            snprintf(msg, MAX_LEN, "%s: next_arg(%s, %c): no argument\n", __FILE__, line ,delim);
            logger(msg);
        }    
        return -1;
    }
    if (debugs[DEBUG_COMM]) {
        snprintf(msg, MAX_LEN, "%s: next_arg(%s, %c): split at %d\n", __FILE__, line ,delim, arg_index);
        logger(msg);
    } 
    return arg_index;
}


void* bulletin_server (int msock) {
    int ssock;                      
    struct sockaddr_in client_addr; 
    socklen_t client_addr_len = sizeof(client_addr); 
   

    pthread_t tt;
    pthread_attr_t ta;
    pthread_attr_init(&ta);
    pthread_attr_setdetachstate(&ta,PTHREAD_CREATE_DETACHED);

    clients = (FILES1*) malloc(sizeof(FILES1) * t_max);
    bulletin_clients = (pthread_t*) malloc (sizeof(pthread_t) * t_max);
    
    memset(clients, 0, t_max * sizeof(FILES1));
    memset(bulletin_clients, 0, sizeof(pthread_t) * t_max );
    for (int i = 0; i < t_max; i++){
        clients[i].resources = &bulletin_res;
        clients[i].syns = &syns;
        clients[i].read = &Read_luck;
        clients[i].write = &Write_lock;
        clients[i].read_num = &number_read;
        if (debug_opt){
            clients[i].read_delay = 3;
            clients[i].Known_time = 6;
        }else{
            clients[i].read_delay = 0;
            clients[i].Known_time = 0;
        }
        
    }
    bulletin_res.set_filename(FilesBB);

    char msg[MAX_LEN];

    for (int i = 0; i < t_max; i++)
        pthread_create(&bulletin_clients[i], NULL, transfer_client, &clients[i]);

    while (1) {
        
        ssock = accept(msock, (struct sockaddr*)&client_addr, &client_addr_len);
        if (ssock < 0) {
            if (errno == EINTR)
                continue;
            snprintf(msg, MAX_LEN, "%s: bulletin server accept: %s\n", __FILE__, strerror(errno));
            logger(msg);
            return 0;
        }

        bool check = false;
        for (int i = 0;(i < t_max) && !check;i++){
            if (pthread_mutex_trylock(&clients[i].process_lock) == 0){
                clients[i].Sok = ssock;
                check = 1;
                pthread_cond_signal(&clients[i].sig_lock);
                pthread_mutex_unlock(&clients[i].process_lock);
            }
        }
        
        if (!check){
            
            snprintf(msg, MAX_LEN,"FAIL 2 Bulletin server is busy, try again later\n");
            send(ssock, msg, strlen(msg),0);
            close(ssock);
        }

    }
    for (int i = 0; i < syns.get_peers_num(); i++)
        syns.close_connection(i);

    return NULL;
}

void* replica_server_thread(int msock){
    
    int ssock;
    struct sockaddr_in client_addr; 
    socklen_t client_addr_len = sizeof(client_addr); 
    syns.init();
    for (int i = 0; i < t_max; i++){
        Snk_thr_resr = (sync_thread*) malloc (20 * sizeof(sync_thread));
        rcr1 = (pthread_t*) malloc (20 * sizeof(pthread_t));
    }
    
    while (1) {
        
       
        ssock = accept(msock, (struct sockaddr*)&client_addr, &client_addr_len);
        if (ssock < 0) {
            if (errno == EINTR)
                continue;
            printf("%s: bulletin server accept: %s\n", __FILE__, strerror(errno));
            return 0;
        }

        int i = 0;
        int check = 0;
        for (;(i < t_max) && !check;i++){
            if (pthread_mutex_trylock(&Snk_thr_resr[i].process) == 0){
                Snk_thr_resr[i].sock = ssock;
                Snk_thr_resr[i].resource = &bulletin_res;
                Snk_thr_resr[i].read = &Read_luck;
                Snk_thr_resr[i].write = &Write_lock;
                Snk_thr_resr[i].debug = debug_opt;
                Snk_thr_resr[i].daemon = Duuyuh;
                Snk_thr_resr[i].log_file = LogFiles;
                check = 1;
                
                pthread_create(&rcr1[i], NULL, sync_receiver, &Snk_thr_resr[i]);
            }
            
        }
        
    }

    return NULL;
}


int main (int argc, char** argv, char** envp) {
  
    int bp = 9000;
    int sp = 10000;
    const int qlen = 32;            
    sprintf(progname, "%s", argv[0]);
    debug_opt = 0;
   
    t_max = 20;

    char msg[MAX_LEN]; 

  
    pthread_mutex_init(&logger_mutex, 0);

   
    extern char *optarg;
    int opt;
    Duuyuh = true;  
    bool b, c, T, p, s, f, d, peers_check;
    b = c = T = p = s = f = d = peers_check = false;

    while ( (opt = getopt(argc, argv, "b:c:T:p:s:fd") ) != -1) {
        switch ((char)opt) {

            case 'b':
                b = 1;
                sprintf(FilesBB, "%s", optarg);
                break;

            case 'T':
                T = 1;
                t_max = atoi(optarg);
                break;

            case 'p':
                p = 1;
                bp = atoi(optarg);
                break;

            case 's':
                s = 1;
                sp = atoi(optarg);
                break;

            case 'f':
                f = 1;
                Duuyuh = false;
                break;

            case 'd':
                d = 1;
                debug_opt = 1;
                printf("Debug mode: will delay IO\n");
                break;

            case 'c':
                c = 1;
                sprintf(Confuguration, "%s", optarg);
                break;

        }
    }

    for (int i = argc - 1; i >= 1; i--){
        char var[20];
        int port = 0;
        sscanf(argv[i], "%[^:]%*[:]%d", var, &port);
        if (port){
            if (debug_opt) 
                cout << "PEERS: " << var << ", " << port << endl;
            if (!strcmp(var, "localhost"))
                sprintf(var, "127.0.0.1");
            syns.push_peer(var, port);
            peers_check = 1;
        }
        
    }

   
    FILE* pfile;
    char buffer[100];
    char option[100];
    pfile = fopen(Confuguration, "rw+");
    while(fscanf(pfile, "%[^=]%*[=]", option) != EOF){
        int result;
       
        if ( strcmp(option, "THMAX") == 0 ){
            fscanf(pfile, "%d", &result);
            if (!T){
                t_max = result;
            }
            if (debug_opt)
                cout << "Tmax: " << t_max << endl;
        }else if ( strcmp(option, "BBPORT") == 0 ){
            fscanf(pfile, "%d", &result);
            if (!p){
                bp = result;
            }
            if (debug_opt)
                cout << "bport: " << bp << endl;
        }else if ( strcmp(option, "BBFILE") == 0 ){
            char str[100];
            if (b){
                fscanf(pfile, "%[^\n]", str);
            }else{
                fscanf(pfile, "%[^\n]", FilesBB);
            }
            if (debug_opt)
                cout << "bbfile: " << FilesBB << endl;
        }else if ( strcmp(option, "SYNCPORT") == 0 ){
            fscanf(pfile, "%d", &result);
            if (!s){
                sp = result;
            }
            if (debug_opt)
                cout << "sport: " << sp << endl;
        }else if ( strcmp(option, "DAEMON") == 0 ){
            char var[10];
            result = fscanf(pfile, "%[^\n]", var);
            if (!f){
                if ( (!strcmp(var, "true")) || (!strcmp(var, "1")) ){
                    Duuyuh = true;
                }
                if ( (!strcmp(var, "false")) || (!strcmp(var, "0")) ){
                    Duuyuh = false;
                }
            }
            if (debug_opt)
                cout << "detach: " << Duuyuh << endl;
        }else if ( strcmp(option, "DEBUG") == 0 ){
            char var[10];
            result = fscanf(pfile, "%[^\n]", var);
            if (!d){
                if ( (!strcmp(var, "true")) || (!strcmp(var, "1")) ){
                    debug_opt = 1;
                }
                if ( (!strcmp(var, "false")) || (!strcmp(var, "0")) ){
                    debug_opt = 0;
                }
            }
            if (debug_opt)
                cout << "debug: " << debug_opt << endl;
        }else if ( strcmp(option, "PEERS") == 0 ){
            char var[20];
            int port;
            while( result = fscanf(pfile, "%[^:\n]%*[:]%d%*[ ]", var, &port) ){
                if (debug_opt && !peers_check) 
                    cout << "PEERS: " << var << ", " << port << endl;
                if (!strcmp(var, "localhost"))
                    sprintf(var, "127.0.0.1");
                if (!peers_check)
                    syns.push_peer(var, port);
            }
        }
        fscanf(pfile, "%*[\n]");

    }
    
   
    fclose(pfile);
 

    t_incr = t_max;
    r_servers_count = 0;
    r_servers = (replica_server*)malloc(sizeof(replica_server)*5);

    syns.setconfig(debug_opt, Duuyuh, LogFiles);

    printf("process ID of current process : %d\n", getpid());
    
    Bdk1 = passivesocket(bp,qlen);
    if (Bdk1 < 0) {
        perror("bulletin server passivesocket");
        return 1;
    }
    printf("bulletin server up and listening on port %d\n", bp);
    
    Sdk1 = passivesocket(sp,qlen);
    if (Sdk1 < 0) {
        perror("Synchronization server passivesocket");
        return 1;
    }
    printf("Synchronization server up and listening on port %d\n", sp);
    
    

    signal(SIGHUP,  sigup_handler);
    signal(SIGQUIT,  sigquit_handler);
   
    if (Duuyuh) {
        
        umask(0177);

       
        signal(SIGINT,  SIG_IGN);
        signal(SIGTERM, SIG_IGN);
        signal(SIGALRM, SIG_IGN);
        signal(SIGSTOP, SIG_IGN);

      
        setpgid(getpid(),0);

      
        int hgf1 = fork();
        if (hgf1 < 0) {
            perror("fork");
            return 1;
        }
        if (hgf1 > 0){
            printf("process ID of child process : %d\n", hgf1);
            return 0;  // parent dies peacefully
        }

       
        for (int i = getdtablesize() - 1; i >= 0 ; i--)
            if (i != Sdk1 && i != Bdk1)
                close(i);
        
        int fd = open("/dev/null", O_RDONLY);
       
        fd = open(LogFiles, O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR);
       
        dup(fd);

       
        fd = open("/dev/tty",O_RDWR);
        ioctl(fd,TIOCNOTTY,0);
        close(fd);

       
    }

   
    pthread_t tt;
    pthread_t bs;
    pthread_t rst;
    pthread_attr_t ta;
    pthread_attr_init(&ta);
    pthread_attr_setdetachstate(&ta,PTHREAD_CREATE_DETACHED);

   
    if ( pthread_create(&bs, NULL, (void* (*) (void*))bulletin_server, (void*)Bdk1) != 0 ) {
        snprintf(msg, MAX_LEN, "%s: pthread_create: %s\n", __FILE__, strerror(errno));
        logger(msg);
        return 1;
    }

    alive123 = true;

    
    replica_server_thread(Sdk1);

   
    while (alive123) {
        sleep(30);
    }

    snprintf(msg, MAX_LEN, "%s: all the servers died, exiting.\n", __FILE__);
    logger(msg);
    
    return 1;
}

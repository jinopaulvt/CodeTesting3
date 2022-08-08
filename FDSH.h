

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <libgen.h>
#include <fcntl.h>
#include <unistd.h>

#include "TULS_TCP.h"






typedef enum {
    idle = 1,
    running  = 2,
    recreate  = 3,
    deactivate = 4
}thread_status;


struct client_t {
    int Fedr;    // the communication socket
    char ip[20];   // the (dotted) IP address
    bool Is_busy;
    pthread_t thread;
    pthread_cond_t thread_cond;
    int Osts;
    pthread_mutex_t lock;
    int Offdwn;
};




struct threadpool_t {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  int Incr_t;
  int Max_Isz;
  int OnActive;
  int Cnt;
  int shutdown;
  int started;
  client_t* clients;
};



struct replica_server{
  int sd;   
  char *ip;
  int port;
};



const size_t MAX_LEN = 1024;


int next_arg(const char*, char);


const size_t DEBUG_COMM = 0;
const size_t DEBUG_FILE = 1;
const size_t DEBUG_DELAY = 2;
extern bool debugs[3]; // What to debug


void logger(const char *);


extern pthread_mutex_t logger_mutex;





struct rwexcl_t {    
    pthread_mutex_t mutex;     
    pthread_cond_t can_write;  
    unsigned int reads;        
                               
    unsigned int owners;       
    int fd;                    
                               
    char* name;                 
};


extern rwexcl_t** Frkos1;
extern size_t Frkos_Size;


const int err_nofile = -2;


void* file_client (client_t*);



void* replica_sync_request_recieve(int);

void update_replica_servers(replica_server*);

void sync_with_server(int);




const int err_exec = 0xFF;

void* shell_client(client_t*);


#include "TULS_TCP.h"

int connectbyport(const char* host, const char* port) {
    return connectbyportint(host,(unsigned short)atoi(port));
} 

int connectbyservice(const char* host, const char* service){
    struct servent* sinfo = getservbyname(service,"tcp"); 
    if (sinfo == NULL)
        return err_proto;
    return connectbyportint(host,(unsigned short)sinfo->s_port);
}

int connectbyportint(const char* host, unsigned short port) {
    struct hostent *hinfo;        
    struct sockaddr_in sin;      
    int	sd;                    
    const int type = SOCK_STREAM; 

    memset(&sin, 0, sizeof(sin)); 
    sin.sin_family = AF_INET;

   
    hinfo = gethostbyname(host);
    if (hinfo == NULL)
        return err_host;
    memcpy(&sin.sin_addr, hinfo->h_addr, hinfo->h_length);

    sin.sin_port = (unsigned short)htons(port);

  
    sd = socket(PF_INET, type, 0);
    if ( sd < 0 )
        return err_sock;

  
    int rc = connect(sd, (struct sockaddr *)&sin, sizeof(sin));
    if (rc < 0) {
        close(sd);
        return err_connect;
    }

  
    return sd;
}

int passivesocketstr(const char* port, int backlog) {
    return passivesocket((unsigned short)atoi(port), backlog);
}

int passivesocketserv(const char* service, int backlog) {
    struct servent* sinfo = getservbyname(service,"tcp");  
    if (sinfo == NULL)
        return err_proto;
    return passivesocket((unsigned short)sinfo->s_port, backlog);
}


int passivesockaux(unsigned short port, int backlog, unsigned long int ip_addr) {
    struct sockaddr_in sin;       
    int	sd;                        
    const int type = SOCK_STREAM; 

    memset(&sin, 0, sizeof(sin)); 
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(ip_addr);

    sin.sin_port = (unsigned short)htons(port);

    
    sd = socket(PF_INET, type, 0);
    if ( sd < 0 )
        return err_sock;

   
    int reuse = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

   
    if ( bind(sd, (struct sockaddr *)&sin, sizeof(sin)) < 0 ) {
        close(sd);
        return err_bind;
    }
  
    if ( listen(sd, backlog) < 0 ) {
        close(sd);
        return err_listen;
    }

   
    return sd;
}

int passivesocket(unsigned short port, int backlog) {
    return passivesockaux(port, backlog, INADDR_ANY);
}

int controlsocket(unsigned short port, int backlog) {
    return passivesockaux(port, backlog, INADDR_LOOPBACK);
}

int recv_nonblock (int sd, char* buf, size_t max, int timeout) {
    struct pollfd pollrec;
    pollrec.fd = sd;
    pollrec.events = POLLIN;
  
    int polled = poll(&pollrec,1,timeout);

    if (polled == 0)
        return recv_nodata;
    if (polled == -1)
        return -1;

    return recv(sd,buf,max,0);
}

int readline(const int fd, char* buf, const size_t max) {
    size_t i;
    int begin = 1;

    for (i = 0; i < max; i++) {
        char tmp;
        int what = read(fd,&tmp,1);
        if (what == -1)
            return -1;
        if (begin) {
            if (what == 0)
                return recv_nodata;
            begin = 0;
        }
        if (what == 0 || tmp == '\n') {
            buf[i] = '\0';
            return i;
        }
        buf[i] = tmp;
    }
    buf[i] = '\0';
    return i;
}

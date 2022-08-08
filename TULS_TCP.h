

#ifndef __TCP_UTILS_H
#define __TCP_UTILS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>






const int err_host    = -1;
const int err_sock    = -2;
const int err_connect = -3;
const int err_proto   = -4;
const int err_bind    = -5;
const int err_listen  = -6;





int connectbyport(const char* host, const char* port);


int connectbyservice(const char* host, const char* service);


int connectbyportint(const char* host, unsigned short port);



int passivesocketstr(const char* port, int backlog);


int passivesocketserv(const char* service, int backlog);


int passivesocket(unsigned short port, int backlog);


int controlsocket(unsigned short port, int backlog);



const int recv_nodata = -2;

int recv_nonblock (int sd, char* buf, size_t max, int timeout);


int readline(int fd, char* buf, size_t max);

#endif /* __TCP_UTILS_H */


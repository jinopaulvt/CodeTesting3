
#include <libgen.h>
#include <stdio.h>

#include "TULS_TCP.h"

int main (int argc, char** argv) {
    const int ALEN = 1024;
    char req[ALEN];
    char ans[ALEN];

    if (argc != 3) {
        printf("Usage: %s  host port\n", basename(argv[0]));
        return 1;
    }

    int sd = connectbyport(argv[1],argv[2]);
    if (sd == err_host) {
        printf("Unable to locate host %s\n", argv[1]);
        return 1;
    }
    if (sd < 0) {
        perror("connectbyport");
        return 1;
    }
 

    printf("Connected to %s  on port %s\n", argv[1], argv[2]);
    while (1) {
        int n;

        while ((n = recv_nonblock(sd,ans,ALEN-1,10)) != recv_nodata) {
            if (n == 0) {
                shutdown(sd, SHUT_RDWR);
                close(sd);
                printf("Connection closed by %s\n", argv[1]);
                return 0;
            }
            ans[n] = '\0';
            printf("%s",ans);
            fflush(stdout);
        }

        
        printf("%s%% ", basename(argv[0]));
        fflush(stdout);
        if (fgets(req,ALEN,stdin) == 0) {
            printf("\n");
            return 0;
        }
        if(strlen(req) > 0 && req[strlen(req) - 1] == '\n')
            req[strlen(req) - 1] = '\0';
        send(sd,req,strlen(req),0);
        send(sd,"\n",1,0);
    
        
        n = readline(sd,ans,ALEN-1);
        printf("%s\n", ans);
    }
}


#include <stdio.h>
#include <string.h>

#define GV_IMPLEMENTATION
#include "../gv.h"


int main(int argc, const char **argv)
{
    gvSockStartup();
    
    GvSocket listen_sock;
    GvSocket conn_sock;

    char buff[512];
    struct sockaddr_in serv_addr;
    struct sockaddr conn_addr;

    socklen_t conn_addr_size;
    
    gv_memset(&serv_addr, 0, sizeof(serv_addr));
    gv_memset(&conn_addr, 0, sizeof(conn_addr));
    gv_memset(buff, 0, sizeof(buff));

    listen_sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(80);

    bind(listen_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr));

    listen(listen_sock, 10);

    while (1) {
        conn_addr_size = sizeof(conn_addr);
        
        conn_sock = accept(listen_sock, &conn_addr, &conn_addr_size);

        if (conn_sock == GVSOCK_INVALID) {
            fprintf(stderr, "error: invalid socket\n");
            continue;
        }

        printf("info: got connection\n");

        const char *response;

        int r;
        do {
            r = recv(conn_sock, buff, sizeof(buff), 0); /* for now we dont care about the request */
            char test[64];
            sscanf(buff, "GET %s\n", test);
            if (strcmp(test, "/") == 0 || strlen(test) == 2) {
                response = "Hello Stranger!";
            } else {
                response = "WTF?!";
            }
        } while(r == sizeof(buff));

        if (r == 0) {
            fprintf(stderr, "error: connection lost\n");
            continue;
        }

        r = 0;

        gv_memset(buff, 0,  sizeof(buff));
        r += sprintf(buff, "HTTP/1.1 200 OK\n\n%s", response);
        
        send(conn_sock, buff, r, 0);
        gvSocketClose(conn_sock);
    }

    gvSocketClose(listen_sock);

    gvSockCleanup();
    return 0;
}
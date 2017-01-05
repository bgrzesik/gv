
#include <stdio.h>
#include <locale.h>
#include <errno.h>

#define GV_SOCK_CONSTRUCTOR
#define GV_IMPLEMENTATION
#include "../gv.h"

#define is_http_separator(buff, i, max) ((i + 3) < max && buff[i] == '\r' && buff[i + 1] == '\n' && buff[i + 2] == '\r' && buff[i + 3] == '\n')

int main(int argc, const char **argv)
{
    const char *hostname = "motherfuckingwebsite.com";
    struct hostent *out = gethostbyname(hostname);

    if (!out) {
        fprintf(stderr, "error: host not found\n");
        return 1;
    }

    gvsock_t sock = socket(AF_INET, SOCK_STREAM, 0); 

    if (sock == GVSOCK_INVALID) {
        fprintf(stderr, "error: invalid socket\n");
        return 3;
    }
    
    struct sockaddr_in addr;
    gv_memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr = *((struct in_addr **) out->h_addr_list)[0];

    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) == GVSOCK_ERROR) {
        fprintf(stderr, "error: unable to connect (%d)\n", errno);
        return 2;
    }

    char buff[512];
    char *pbuff = buff;
    pbuff += sprintf(pbuff, "GET / HTTP/1.1\n");
    pbuff += sprintf(pbuff, "Accept: text/plain; q=0.5, text/html; charset=UTF-8\n");
    pbuff += sprintf(pbuff, "Host: %s\n", hostname);
    pbuff += sprintf(pbuff, "Accept-Charset: UTF-8\n");
    pbuff += sprintf(pbuff, "\n");


    printf("%s\n===================================\n", buff);
    send(sock, buff, strlen(buff), 0);
    
    
    size_t num_bytes;
    size_t content_len;
    size_t read = 0;

    int headers_gone = 0;
    int i;

    do {
        num_bytes = recv(sock, buff, sizeof(buff), 0);
        
        if (!headers_gone) {
            for (i = 0; i < num_bytes; i++) {
                if (is_http_separator(buff, i, num_bytes)) {
                    headers_gone = 1;
                    i += 3;
                    break;
                } else {
                    putchar(buff[i]);
                }

                size_t tmp;
                if (sscanf(&buff[i], "Content-Length: %zu\r\n", &tmp) != 0) {
                    content_len = tmp;
                }
            }
            if (!headers_gone) {
                continue;
            }
        }
        
        for (; i < num_bytes; i++) {
            putchar(buff[i]);
            read++;
        }

        i = 0;
    } while (num_bytes != 0 && read != content_len);

close:
    gvsock_close(sock);

    return 0;
}
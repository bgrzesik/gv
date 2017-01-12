
#define GV_SOCK_CONSTRUCTOR
#define GV_IMPLEMENTATION
#include <gv.h>

#include <Winsock2.h>
#include <Mstcpip.h>
#include <ws2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")

#define WIN32_LEAN_AND_MEAN
#include <wtypes.h>

#pragma pack(push, 1)
/* header */
struct GvIPHeader {
    UCHAR  ip_hdr_len       : 4;
    UCHAR  ip_ver           : 4;
    UCHAR  ip_tos           : 8;
    USHORT ip_len           : 16; /* int */
    USHORT ip_id            : 16;
    
    UCHAR  ip_frag_offset   : 5;
    UCHAR  ip_more_fragment : 1;
    UCHAR  ip_dont_fragment : 1;
    UCHAR  ip_reserved_zero : 1;
    UCHAR  ip_frag_offset1  : 8;  /* int */

    UCHAR  ip_ttl           : 8;
    UCHAR  ip_protocol      : 8;
    USHORT ip_checksum      : 16; /* int */

    UINT   ip_srcaddr       : 32; /* int */
    UINT   ip_dstaddr       : 32; /* int */
};

struct GvUDPHeader {
    USHORT udp_srcport      : 16;
    USHORT udp_dstport      : 16;
    USHORT udp_len          : 16;
    USHORT udp_checksum     : 16;
};
#pragma pack(pop)

GV_STATIC_ASSERT(sizeof(struct GvIPHeader) == 21);

#ifdef _WIN32
#define close closesocket
#endif

#define NLOCALHOST (0x0100007f) /* 127.0.0.1 */

/* test server running in another thread */
DWORD backgroundServer(void *param) {
    HANDLE event = (HANDLE) param;

    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0) {
        int wsa_err = WSAGetLastError();
        __debugbreak();
    }

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = NLOCALHOST;
    saddr.sin_port = htons(6969);
    
    int addr_in_size = sizeof(saddr);
    int result;

    if (result = bind(s, &saddr, sizeof(saddr)) != 0) {
        int wsa_err = WSAGetLastError();
        __debugbreak();
    }

    SetEvent(event);

    struct addrinfo from;
    int addr_size = sizeof(from);

    static char buff[255];

    int received = recvfrom(s, buff, sizeof(buff), 0, &from, &addr_size);
    if (received < 0) {
        int wsa_err = WSAGetLastError();
        __debugbreak();
    }

    OutputDebugStringA("[S] RECEIVED\n");
    OutputDebugStringA(buff);
    OutputDebugStringA("\n[S] END RECEIVED\n");

    addr_size = sizeof(from);

    struct sockaddr_in to;
    to.sin_family = AF_INET;
    to.sin_port = htons(9690);
    to.sin_addr.s_addr = NLOCALHOST;

    int to_size = sizeof(to);

    static const char response[] = "Hi dumpass!";

    int sent = sendto(s, response, sizeof(response), 0, &to, to_size);
    if (sent < 0) {
        int wsa_err = WSAGetLastError();
        __debugbreak();
    }

    OutputDebugStringA("[S] SEND BACK\n");
    
    Sleep(100);
    
    sent = sendto(s, response, sizeof(response), 0, &to, to_size);
    if (sent < 0) {
        int wsa_err = WSAGetLastError();
        __debugbreak();
    }

    OutputDebugStringA("[S] SEND BACK\n");

    WaitForSingleObject(event, INFINITE);
    close(s);
}

int WinMain(HINSTANCE hinstance, HINSTANCE hprev, char *cmd_line, int cmd_show) {
    HANDLE event = CreateEvent(NULL, TRUE, FALSE, NULL);
    HANDLE bgServer = CreateThread(NULL, 0,  &backgroundServer, event, 0, NULL);
    
    WaitForSingleObject(event, INFINITE);
    ResetEvent(event);

    SOCKET s = WSASocketA(AF_INET, SOCK_RAW, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);

    if (s == INVALID_SOCKET) {
        int wsa_err = WSAGetLastError();
        __debugbreak();
    }

    int optval = 1;
    if (setsockopt(s, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(optval)) == SOCKET_ERROR) {
        int wsa_err = WSAGetLastError();
        __debugbreak();
    }

    struct sockaddr_in bind_addr;
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(9696);
    bind_addr.sin_addr.s_addr = NLOCALHOST;

    if (bind(s, &bind_addr, sizeof(bind_addr)) < 0) {
        int wsa_err = WSAGetLastError();
        __debugbreak();
    }

    int rcv = RCVALL_IPLEVEL;
    DWORD b = 0;
    int ret;
    if (ret = WSAIoctl(s, SIO_RCVALL, &rcv, sizeof(rcv), 0, 0, &b, 0, 0) < 0) {
        int wsa_err = WSAGetLastError();
        __debugbreak();
    }

    static const char msg[] = "Hello Motherfuckers!";

    char buff[255];
    ZeroMemory(&buff, sizeof(buff));

    struct GvIPHeader *ip_hdr = (struct GvIPHeader *) buff;
    ip_hdr->ip_ver = 4;
    ip_hdr->ip_hdr_len = 5;
    ip_hdr->ip_tos = 0;
    ip_hdr->ip_len = htons(sizeof(struct GvIPHeader) + sizeof(struct GvUDPHeader) + sizeof(msg));
    ip_hdr->ip_id = htons(2);
    ip_hdr->ip_frag_offset = 0;
    ip_hdr->ip_frag_offset1 = 0;
    ip_hdr->ip_reserved_zero = 0;
    ip_hdr->ip_dont_fragment = 1;
    ip_hdr->ip_more_fragment = 0;
    ip_hdr->ip_ttl = 8;
    ip_hdr->ip_protocol = IPPROTO_UDP;
    ip_hdr->ip_srcaddr = NLOCALHOST;
    ip_hdr->ip_dstaddr = NLOCALHOST;
    ip_hdr->ip_checksum = 0;

    struct GvUDPHeader *udp_hdr = (struct GvUDPHeader *) (buff + sizeof(struct GvIPHeader));
    udp_hdr->udp_srcport = htons(9696);
    udp_hdr->udp_dstport = htons(6969);
    udp_hdr->udp_checksum = 0;
    udp_hdr->udp_len = htons(sizeof(struct GvUDPHeader) + sizeof(msg));

    char *buff_data = (char *) (buff + sizeof(struct GvIPHeader) + sizeof(struct GvUDPHeader));
    memcpy(buff_data, msg, sizeof(msg));


    struct sockaddr_in dst;
    dst.sin_family = AF_INET;
    dst.sin_port = htons(0);
    dst.sin_addr.s_addr = NLOCALHOST;

    int sent = sendto(s, buff, sizeof(struct GvIPHeader) + sizeof(struct GvUDPHeader) + sizeof(msg), 0, &dst, sizeof(dst));
    if (sent < 0) {
        int wsa_err = WSAGetLastError();
        __debugbreak();
    }

    struct sockaddr_in from;
    int from_size = sizeof(from);
    int received;

    static char rbuff[2048];
    
     do {
        received = recvfrom(s, rbuff, sizeof(rbuff), 0, &from, &from_size);

        if (received < 0) {
            int wsa_err = WSAGetLastError();
            __debugbreak();
            break;
        }

        ip_hdr = (struct GvIPHeader *) rbuff;
        udp_hdr = (struct GvUDPHeader *) (rbuff+ sizeof(struct GvIPHeader));
        const char *data = rbuff + sizeof(struct GvIPHeader) + sizeof(struct GvUDPHeader);

        UINT srcaddr = ntohl(ip_hdr->ip_srcaddr);
        UINT dstaddr = ntohl(ip_hdr->ip_dstaddr);
        USHORT srcport = ntohs(udp_hdr->udp_srcport);
        USHORT dstport = ntohs(udp_hdr->udp_dstport);

        OutputDebugStringA("Processing packets\n");

        if (ip_hdr->ip_protocol == IPPROTO_UDP && dstport == 9690) {
            OutputDebugStringA("RAW GOT SOMETHING!\n");
            OutputDebugStringA(data);
            OutputDebugStringA("\nRAW END\n");
//            break;
        }
    } while (received > 0);

    close(s);

    SetEvent(event);
    WaitForSingleObject(bgServer, INFINITE);
    CloseHandle(event);

    ExitProcess(0);
    return 0;
}

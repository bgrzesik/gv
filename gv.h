/*
	gv.h - private code - Bartlomiej's Utilities


ABOUT:
	This is simple utilities for code writing


 */


#ifndef __GV_H__
#define __GV_H__

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__GNUC__) && !(defined(__llvm__) || defined(__clang__))
	//#error "GNU compiler and clang is only supported at the moment"
#endif

#if (defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__)) && !defined(GV_SYS_64BIT)
	#define GV_SYS_64BIT
#elif !defined(GV_SYS_32BIT)
	#define GV_SYS_32BIT
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ > 19901L)
	#include <stdint.h>

	#ifndef gvu8_t
		#define gvu8_t uint8_t
	#endif
	
	#ifndef gvu16_t
		#define gvu16_t uint16_t
	#endif

	#ifndef gvu32_t
		#define gvu32_t uint32_t
	#endif

	#ifndef gvu64_t
		#define gvu64_t uint64_t
	#endif


	#ifndef gvs8_t
		#define gvs8_t int8_t
	#endif
	
	#ifndef gvs16_t
		#define gvs16_t int16_t
	#endif

	#ifndef gvs32_t
		#define gvs32_t int32_t
	#endif

	#ifndef gvs64_t
		#define gvs64_t int64_t
	#endif

	#ifndef gvintptr_t
		#define gvintptr_t intptr_t
	#endif

	#ifndef gvptrdiff_t
		#define gvptrdiff_t ptrdiff_t
	#endif

#else

	#ifndef gvu8_t
		#define gvu8_t unsigned char
	#endif
	
	#ifndef gvu16_t
		#define gvu16_t unsigned short
	#endif

	#ifndef gvu32_t
		#define gvu32_t unsigned int
	#endif

	#ifndef gvu64_t
		#define gvu64_t unsigned long long
	#endif


	#ifndef gvs8_t
		#define gvs8_t char
	#endif
	
	#ifndef gvs16_t
		#define gvs16_t short
	#endif

	#ifndef gvs32_t
		#define gvs32_t int
	#endif

	#ifndef gvs64_t
		#define gvs64_t long long
	#endif

	#ifdef GV_SYS_64BIT
		#ifndef gvintptr_t
			#define gvintptr_t gvu64_t 
		#endif

		#ifndef gvptrdiff_t
			#define gvptrdiff_t gvu64_t
		#endif
	#else
		#ifndef gvintptr_t
			#define gvintptr_t gvu32_t
		#endif

		#ifndef gvptrdiff_t
			#define gvptrdiff_t gvu32_t
		#endif
	#endif
#endif


#ifndef gvmin
	#define gvmin(a, b) ({ __typeof__(a) _a = (a); __typeof__(b) _b = (b); (_a < _b ? _a : _b); })
#endif

#ifndef gvmax
	#define gvmax(a, b) ({ __typeof__(a) _a = (a); __typeof__(b) _b = (b); (_a > _b ? _a : _b); })
#endif

#ifndef GV_STATIC_ASSERT
	#define GV_STATIC_ASSERT(...) struct { int: (!!(__VA_ARGS__)); }
#endif


#ifdef _WIN32
	#pragma comment (lib, "Ws2_32.lib")
	#pragma comment (lib, "Mswsock.lib")
	#pragma comment (lib, "AdvApi32.lib")

	#include <WinSock2.h>
	#include <ws2tcpip.h>
	#include <windows.h>

	#ifndef gvdl_open
		#define gvdl_open(lib) LoadLibrary((lib));
	#endif

	#ifndef gvdl_symbol
		#define gvdl_symbol(lib, sym) ((void *) GetProcAddress((lib), (sym)))
	#endif

	#ifdef gvdl_close
		#define gvdl_close(lib) FreeLibrary((lib))
	#endif

	#ifndef gvsock_init
		#define gvsock_init() ({ WSADATA wsa_data; WSAStartup(MAKEWORD(2, 2), &wsa_data); });
	#endif

	#ifndef gvsock_cleanup
		#define gvsock_cleanup() WSACleanup()
	#endif

	#ifndef gvsock_close
		#define gvsock_close(sock) closesocket(sock) 
	#endif

	#ifndef GVSOCK_INVALID
		#define GVSOCK_INVALID INVALID_SOCKET
	#endif
	
	#ifndef GVSOCK_ERROR
		#define GVSOCK_ERROR SOCKET_ERROR
	#endif

	#ifndef GVSOCK_SD_RECV
		#define GVSOCK_SD_RECV SD_RECEIVE
	#endif

	#ifndef GVSOCK_SD_SEND
		#define GVSOCK_SD_SEND SD_SEND
	#endif

	#ifndef GVSOCK_SD_BOTH
		#define GVSOCK_SD_BOTH SD_BOTH
	#endif

	#ifndef gvsock_t
		#define gvsock_t SOCKET
	#endif

#else
	#include <dlfcn.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <unistd.h>

	#ifndef gvdl_open
		#define gvdl_open(lib) dlopen((lib), RTLD_LAZY)
	#endif

	#ifndef gvdl_symbol
		#define gvdl_symbol(lib, sym) dlsym((lib), (sym))
	#endif

	#ifdef gvdl_close
		#define gvdl_close(lib) dlclose((lib))
	#endif

	#ifndef gvsock_init
		#define gvsock_init() ({ WSADATA wsa_data; WSAStartup(MAKEWORD(2, 2), &wsa_data); });
	#endif

	#ifndef gvsock_cleanup
		#define gvsock_cleanup() do {} while(0)
	#endif

	#ifndef gvsock_close
		#define gvsock_close(sock) close(sock) 
	#endif

	#ifndef GVSOCK_INVALID
		#define GVSOCK_INVALID (-1)
	#endif

	#ifndef GVSOCK_ERROR
		#define GVSOCK_ERROR (-1)
	#endif
	
	#ifndef GVSOCK_SD_RECV
		#define GVSOCK_SD_RECV SHUT_RD
	#endif

	#ifndef GVSOCK_SD_SEND
		#define GVSOCK_SD_SEND SHUT_WR
	#endif

	#ifndef GVSOCK_SD_BOTH
		#define GVSOCK_SD_BOTH SHUT_RDWR
	#endif

	#ifndef gvsock_t
		#define gvsock_t int
	#endif

#endif



#ifdef __cplusplus
}
#endif

#endif

/*
	IMPLEMENTATION
 */
#ifdef GV_IMPLEMENTATION
#endif


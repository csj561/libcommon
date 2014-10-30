#ifndef _SOCKETS_H_
#define _SOCKETS_H_
#if defined _WIN32 || defined _WIN64
#define _ISWINDOWS
#include <WINSOCK2.H>
#pragma comment(lib,"ws2_32.lib")
#else
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#endif
#include <string>

#ifndef _ISWINDOWS
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
#define closesocket close
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#else
typedef int socklen_t;
#endif
typedef unsigned char uint8;
typedef unsigned short uint16;
class Socketer
{
		
protected:
	SOCKET fd;
	SOCKADDR_IN serv_addr;
	void init(SOCKET _fd);
public:
	Socketer(SOCKET _fd);
	Socketer();
	Socketer(const Socketer &);
	Socketer &operator=(const Socketer &);
	void Close(){if(fd!=INVALID_SOCKET) {closesocket(fd);fd=INVALID_SOCKET;}}
	SOCKET getSocket()const{return fd;}
	std::string get_remote_ip();
	uint16 get_remote_port();
	virtual int send(const void *buf,size_t size) const=0;
	virtual int recv(void *buf,size_t size) const=0;
#ifdef _ISWINDOWS
		int setBlock(bool block){int i;if(block) i=0;else i=1;return ioctlsocket(fd,FIONBIO,(u_long*)&i);}
#else
		int setBlock(bool block){int flags=fcntl(fd,F_GETFL,0);if(block) flags &= ~O_NONBLOCK;else flags|=O_NONBLOCK;return fcntl(fd,F_SETFL,flags);}
#endif
	virtual ~Socketer(){if(fd!=INVALID_SOCKET)closesocket(fd);}
};
class udpSender:public Socketer
{
	public:
		udpSender(){}
		udpSender(SOCKET _fd):Socketer(_fd){}
		udpSender(const std::string &ip,const uint16 &dest_port);
		udpSender(const std::string &ip,const uint16 &dest_port,const uint16 &local_port);
		int send(const void *buf,size_t size) const;
		int recv(void *buf,size_t size)const;
		int recv(void *buf,size_t size,struct sockaddr_in& form_addr, size_t &formlen)const;
		int recv(void *buf,size_t size,std::string & form_addr, uint16 &from_port)const;
		int send(const std::string &ip,uint16 port,const void *buf,size_t size) const;
		int send(const SOCKADDR_IN &to_addr,const void *buf,size_t size) const;
};

class tcpSender:public Socketer
{
	public:
		tcpSender(){}
		tcpSender(SOCKET _fd,const SOCKADDR_IN *addr=NULL):Socketer(_fd){if(addr) serv_addr=*addr;}
		tcpSender(const std::string &ip,uint16 serv_port);
		tcpSender(const std::string &ip,uint16 serv_port,uint16 local_port);
		tcpSender(const SOCKADDR_IN &_serv_addr);
		tcpSender(const SOCKADDR_IN &_serv_addr,uint16 local_port);
		int send(const void *buf,size_t size) const;
		int recv(void *buf,size_t size) const;
		int connect();
};

class tcpListener:public Socketer
{
	public:
		tcpListener(){}
		tcpListener(SOCKET _fd):Socketer(_fd){}
		tcpListener(const uint16 &listen_port,int maxConnect=16);
		tcpListener(const std::string &listen_ip,const uint16 &listen_port,int maxConnect=16);
		tcpSender accept(SOCKADDR_IN* addr=NULL,int* addrlen=NULL);
		tcpSender accept(std::string &ip,uint16 &port);
		int send(const void *buf,size_t size) const{return -1;};
		int recv(void *buf,size_t size) const{return -1;};
};
#endif


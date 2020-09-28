#ifndef _SOCKET_H_
#define _SOCKET_H_
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <string>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

class TCPSocket{
    protected:
        TCPSocket();
        virtual ~TCPSocket();
    protected:
        int sockfd;
    protected:
        bool create();
        bool bind(uint16_t port, const char* ip) const;
        bool listen() const;
        bool accept(TCPSocket &ClinetSocket) const;
        bool connect(uint16_t port, const char* ip) const;
        bool isValid() const{return sockfd != -1;}
    public:
        bool close();
        int getfd() const{return sockfd;}
};

class TCPClient : public TCPSocket{
    private:
        struct Packet{
            unsigned int msglen;
            char         text[1024];
        };
    public:
        TCPClient(uint16_t port, const char* ip);
        TCPClient();
        TCPClient(int clientfd);
        ~TCPClient();

    size_t send(const std::string& message) const ;
    size_t receive(std::string& message) const ;
    size_t read(void* buf, size_t count) ;
    void write(const void* buf, size_t count) ;
    size_t write(const char* message) ;
};

class TCPServer : public TCPSocket{
    public:
        TCPServer(uint16_t port, const char *ip) ;
        TCPServer(uint16_t port) ;
        ~TCPServer();
        void accept(TCPClient &client) const ;
        TCPClient accept() const ;
};
#endif
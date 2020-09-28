#include "socket.h"

TCPSocket::TCPSocket():sockfd(-1){}
TCPSocket::~TCPSocket(){
    if(isValid())::close(sockfd);
}
bool TCPSocket::create(){
    if(isValid()){return false;}
    if((sockfd = ::socket(AF_INET, SOCK_STREAM, 0)) < 0){return false;}
    return true;
}
bool TCPSocket::bind(uint16_t port, const char* ip) const{
    if(!isValid()){return false;}
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if(ip == nullptr){
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else{
        addr.sin_addr.s_addr = inet_addr(ip);
    }
    if(::bind(sockfd, (const struct sockaddr*)& addr, sizeof(addr)) == -1){return false;}
    return true;
}
bool TCPSocket::listen() const {
    if(!isValid()){return false;}
    if(::listen(sockfd, 0) == -1){
        return false;
    }
    return true;
}
bool TCPSocket::accept(TCPSocket &clientSocket) const {
    if(!isValid()){return false;}
    clientSocket.sockfd = ::accept(this->sockfd, NULL, NULL);
    if(clientSocket.sockfd == -1){
        return false;
    }
    return true;
}
bool TCPSocket::connect(uint16_t port, const char* ip) const {
    if(!isValid()){return false;}
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    auto inaddr = inet_addr(ip);   //判断ip是ip地址，还是域名
    struct hostent *hp;
    if (inaddr != INADDR_NONE)
        memcpy(&addr.sin_addr, &inaddr, sizeof(inaddr));
    else
    {
        hp = gethostbyname(ip);
        if (hp == NULL)
            return -1;
        memcpy(&addr.sin_addr, hp->h_addr, hp->h_length);
    }
    if(::connect(sockfd, (const struct sockaddr* )& addr, sizeof(addr)) == -1){return false;}
    return true;
}
bool TCPSocket::close(){
    if(!isValid()){return false;}
    ::close(sockfd);
    sockfd = -1;
    return true;
}


TCPServer::TCPServer(uint16_t port, const char *ip) {
    if (create() == false)
        std::cerr << "tcp server create error" << std::endl;
    if (bind(port, ip) == false)
        std::cerr << "tcp server bind error" << std::endl;
    if (listen() == false)
        std::cerr << "tcp server listen error" << std::endl;
}

TCPServer::TCPServer(uint16_t port) {
    const char* ip = "0.0.0.0";
    if (create() == false)
        std::cerr << "tcp server create error" << std::endl;
    if (bind(port, ip) == false)
        std::cerr << "tcp server bind error" << std::endl;
    if (listen() == false)
        std::cerr << "tcp server listen error" << std::endl;
}

TCPServer::~TCPServer() {}
void TCPServer::accept(TCPClient &client) const {
    //显式调用基类TCPSocket的accept
    if (!TCPSocket::accept(client))
        std::cerr << "tcp server accept error" << std::endl;
}
TCPClient TCPServer::accept() const {
    TCPClient client;
    if (!TCPSocket::accept(client)){
        std::cerr << "tcp server accept error" << std::endl;
    }
    return client;
}

TCPClient::TCPClient(uint16_t port, const char *ip){
    if (create() == false)
        std::cerr << "tcp client create error" << std::endl;
    if (connect(port, ip) == false){
        std::cerr << "tcp client connect error" << std::endl;
    }
        
}
TCPClient::TCPClient() {}
TCPClient::TCPClient(int clientfd){
    sockfd = clientfd;
}
TCPClient::~TCPClient() {}
/** client端特有的send/receive **/
static ssize_t readn(int fd, void *buf, size_t count);
static ssize_t writen(int fd, const void *buf, size_t count);
 
//send
size_t TCPClient::send(const std::string& message) const {
    Packet buf;
    buf.msglen = htonl(message.length());
    strcpy(buf.text, message.c_str());
    if (writen(sockfd, &buf, sizeof(buf.msglen)+message.length()) == -1)
        std::cerr << "tcp client writen error" << std::endl;
    return message.length();
}
//receive
size_t TCPClient::receive(std::string& message)const {
    //首先读取头部
    Packet buf = {0, 0};
    size_t readBytes = readn(sockfd, &buf.msglen, sizeof(buf.msglen));
    if (readBytes == (size_t)-1)
        std::cerr << "tcp client readn error" << std::endl;
    else if (readBytes != sizeof(buf.msglen))
        std::cerr << "peer connect closed" << std::endl;
 
    //然后读取数据部分
    unsigned int lenHost = ntohl(buf.msglen);
    readBytes = readn(sockfd, buf.text, lenHost);
    if (readBytes == (size_t)-1)
        std::cerr << "tcp client readn error" << std::endl;
    else if (readBytes != lenHost)
        std::cerr << "peer connect closed" << std::endl;
    message = buf.text;
    return message.length();
}
size_t TCPClient::read(void *buf, size_t count) 
{
    ssize_t readBytes = ::read(sockfd, buf, count);
    if (readBytes == -1)
        std::cerr << "tcp client read error" << std::endl;
    return (size_t)readBytes;
}
void TCPClient::write(const void *buf, size_t count) 
{
    if ( ::write(sockfd, buf, count) == -1 )
        std::cerr << "tcp client write error" << std::endl;
}
size_t TCPClient::write(const char *msg) {
    if ( ::write(sockfd, msg, strlen(msg)) == -1 )
        std::cerr << "tcp client write error" << std::endl;
    return strlen(msg);
}

static ssize_t readn(int fd, void *buf, size_t count)
{
    size_t nLeft = count;
    ssize_t nRead = 0;
    char *pBuf = (char *)buf;
    while (nLeft > 0)
    {
        if ((nRead = read(fd, pBuf, nLeft)) < 0)
        {
            //如果读取操作是被信号打断了, 则说明还可以继续读
            if (errno == EINTR)
                continue;
            //否则就是其他错误
            else
                return -1;
        }
        //读取到末尾
        else if (nRead == 0)
            return count-nLeft;
 
        //正常读取
        nLeft -= nRead;
        pBuf += nRead;
    }
    return count;
}
static ssize_t writen(int fd, const void *buf, size_t count)
{
    size_t nLeft = count;
    ssize_t nWritten = 0;
    char *pBuf = (char *)buf;
    while (nLeft > 0)
    {
        if ((nWritten = write(fd, pBuf, nLeft)) < 0)
        {
            //如果写入操作是被信号打断了, 则说明还可以继续写入
            if (errno == EINTR)
                continue;
            //否则就是其他错误
            else
                return -1;
        }
        //如果 ==0则说明是什么也没写入, 可以继续写
        else if (nWritten == 0)
            continue;
        //正常写入
        nLeft -= nWritten;
        pBuf += nWritten;
    }
    return count;
}



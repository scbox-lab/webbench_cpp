#include "socket.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/signal.h>

int main()
{
    signal(SIGPIPE, SIG_IGN);
    TCPClient client(8001, "127.0.0.1");
    std::string msg;
    while (getline(std::cin, msg))
    {
        client.send(msg);
        msg.clear();
        client.receive(msg);
        std::cout << msg << std::endl;
        msg.clear();
    }
}
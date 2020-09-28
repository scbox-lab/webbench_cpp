#include "socket.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/signal.h>

void sigHandler(int signo)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

int main()
{
    signal(SIGCHLD, sigHandler);
    signal(SIGPIPE, SIG_IGN);

    TCPServer server(8001);
    std::string msg;
    while (true)
    {
        TCPClient client = server.accept();
        pid_t pid = fork();
        if (pid == -1)
            std::cerr << "fork error" << std::endl;
        else if (pid > 0)
            client.close();
        else if (pid == 0)
        {
            while (true)
            {
                client.receive(msg);
                std::cout << msg << std::endl;
                client.send(msg);
            }
        }
    }
}
g++ -g -Wall -std=c++17 socket.cpp socket.h server_text.cpp -o server_text
g++ -g -Wall -std=c++17 socket.cpp socket.h client_text.cpp -o client_text

g++ -g -Wall -std=c++17 socket.cpp socket.h webbench.cpp -o webbench
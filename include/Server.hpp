#ifndef SERVER_HPP
#define SERVER_HPP

class IrcCore;

class Server
{
private:
    int         _port;
    int         _listenFd;
    IrcCore&    _core;

    Server();
    Server(const Server& other);
    Server& operator=(const Server& other);

public:
    Server(int port, IrcCore& core);
    ~Server();

    void run();
};

#endif
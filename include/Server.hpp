#ifndef SERVER_HPP
#define SERVER_HPP

#include <poll.h>
#include <vector>
#include <map>

#include "Connection.hpp"

class   IrcCore;
struct  OutgoingMessage;

class Server
{
private:
    int                         _port;
    int                         _listenFd;
    IrcCore&                    _core;

    std::vector<struct pollfd>  _pollFds;
    std::map<int, Connection>   _connections;

    Server();
    Server(const Server& other);
    Server& operator=(const Server& other);

    void    acceptConnection();
    bool    handleRead(int fd);
    bool    handleWrite(int fd);
    void    disconnectClient(int fd);

public:
    Server(int port, IrcCore& core);
    ~Server();

    void    run();
    void    queueMessage(const OutgoingMessage& message);
};

#endif
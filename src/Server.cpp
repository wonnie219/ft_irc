#include <unistd.h>
#include <sys/socket.h>
#include <stdexcept>
#include <fcntl.h>
#include <netinet/in.h>
#include <cstring>
#include <poll.h>
#include <cerrno>
#include <iostream>

#include "Server.hpp"

Server::Server(int port, IrcCore& core)
    : _port(port), _listenFd(-1), _core(core)
{
    //empty
}

Server::~Server() {
    if (_listenFd >= 0)
        close(_listenFd);
}

void Server::run() {
    _listenFd = socket(AF_INET, SOCK_STREAM, 0);

    if (_listenFd == -1)
        throw std::runtime_error("socket() failed");

    int option = 1;

    if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
        throw std::runtime_error("setsockopt() faild");

    if (fcntl(_listenFd, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl() faild");

    struct  sockaddr_in address;
    std::memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(_port);

    if (bind(_listenFd, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) == -1)
        throw std::runtime_error("bind() failded");

    if (listen(_listenFd, SOMAXCONN) == -1)
        throw std::runtime_error("listen() failed");

    struct pollfd   listenPoll;
    
    listenPoll.fd = _listenFd;
    listenPoll.events = POLLIN;
    listenPoll.revents = 0;
    
    while (true) {
        int ready = poll(&listenPoll, 1, -1);

        if (ready == -1) {
            if (errno == EINTR || errno == EAGAIN || errno == ENOMEM)
                continue;
            throw std::runtime_error("poll() failed");
        }
        
        if (listenPoll.revents & (POLLERR | POLLHUP | POLLNVAL))
            throw std::runtime_error("Listening socket error");
            
        if (listenPoll.revents & POLLIN)
            acceptConnection();
    }

    (void)_core;
}

void Server::acceptConnection()
{
    int clientFd = accept(_listenFd, NULL, NULL);
    
    if (clientFd == -1)
    {
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        throw std::runtime_error("accept() failed");
    }
    
    std::cout << "Client connected: fd " << clientFd << std::endl;
    
    close(clientFd);
}
#include <unistd.h>

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
    (void)_port;
    (void)_core;
}
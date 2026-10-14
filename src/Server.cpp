#include <unistd.h>
#include <sys/socket.h>
#include <stdexcept>
#include <fcntl.h>
#include <netinet/in.h>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <utility>

#include "Server.hpp"

Server::Server(int port, IrcCore& core)
    : _port(port), _listenFd(-1), _core(core), _pollFds(), _connections()
{
    //empty
}

Server::~Server() {
    for (std::map<int, Connection>::iterator it = _connections.begin(); it != _connections.end(); it++)
        close(it->first);

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

    int flags = fcntl(_listenFd, F_GETFL, 0);

    if (flags == -1 || fcntl(_listenFd, F_SETFL, flags | O_NONBLOCK) == -1) {
        throw std::runtime_error("Listening fcntl() failed");
    }

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
    
    _pollFds.push_back(listenPoll);

    while (true) {
        int ready = poll(&_pollFds[0], _pollFds.size(), -1);

        if (ready == -1) {
            if (errno == EINTR)
                continue;
            throw std::runtime_error("poll() failed");
        }
        
        for (std::size_t i = 0; i < _pollFds.size();) {
            int fd = _pollFds[i].fd;
            short revents = _pollFds[i].revents;

            if (fd == _listenFd) {
                if (revents & (POLLERR | POLLHUP | POLLNVAL))
                    throw std::runtime_error("Listening socket error");
            
                if (revents & POLLIN)
                    acceptConnection();

                i++;
                continue;
            }

            if (revents & (POLLERR | POLLNVAL)) {
                disconnectClient(fd);
                continue;
            }

            if (revents & POLLIN) {
                if (!handleRead(fd))
                    continue;
            }

            if (revents & POLLHUP) {
                disconnectClient(fd);
                continue;
            }

            ++i;
        }
    }
    (void)_core;
}

void Server::acceptConnection() {
    int clientFd = accept(_listenFd, NULL, NULL);
    
    if (clientFd == -1) {
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        throw std::runtime_error("accept() failed");
    }
    
    int flags = fcntl(clientFd, F_GETFL, 0);

    if (flags == -1 || fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) == -1) {
        close(clientFd);
        throw std::runtime_error("Client fcntl() failed");
    }

    struct pollfd clientPoll;

    clientPoll.fd = clientFd;
    clientPoll.events = POLLIN;
    clientPoll.revents = 0;

    try {
        _connections.insert(std::make_pair(clientFd, Connection(clientFd)));
        _pollFds.push_back(clientPoll);
    }
    catch (...) {
        _connections.erase(clientFd);
        close(clientFd);
        throw;
    }
    std::cout << "Client connected: fd " << clientFd << std::endl;
}

bool Server::handleRead(int fd) {
    char buffer[4096];

    ssize_t received = recv(fd, buffer, sizeof(buffer), 0);

    if (received > 0) {
        std::map<int, Connection>::iterator it = _connections.find(fd);

        if (it == _connections.end()) {
            disconnectClient(fd);
            return false;
        }

        std::string& recvBuffer = it->second.getRecvBuffer();
        const std::size_t maxBufferSize = 64 * 1024;
        std::size_t receivedSize = static_cast<std::size_t>(received);

        if (receivedSize > maxBufferSize || recvBuffer.size() > maxBufferSize - receivedSize) {
            disconnectClient(fd);
            return false;
        }

        recvBuffer.append(buffer, receivedSize);

        std::cout << "Received " << received << " bytes from fd " << fd << std::endl;

        return true;
    }

    if (received == -1 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
        return true;

    disconnectClient(fd);
    return false;
}

void Server::disconnectClient(int fd) {
    for (std::size_t i = 1; i < _pollFds.size(); ++i) {
        if (_pollFds[i].fd == fd) {
            _pollFds.erase(_pollFds.begin() + i);
            break;
        }
    }

    _connections.erase(fd);
    close(fd);

    std::cout << "Client disconnected: fd " << fd << std::endl;
}
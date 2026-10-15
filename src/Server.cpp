#include <unistd.h>
#include <sys/socket.h>
#include <stdexcept>
#include <fcntl.h>
#include <netinet/in.h>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <utility>
#include <csignal>

#include "Server.hpp"
#include "Common.hpp"
#include "IrcCore.hpp"

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
    std::map<int, Connection>::iterator it = _connections.find(fd);

    if (it == _connections.end())
        throw std::logic_error("handleRead(): unknown client fd");

    std::string& recvBuffer = it->second.getRecvBuffer();
    char buffer[4096];

    while (true) {
        ssize_t received = recv(fd, buffer, sizeof(buffer), 0);

        if (received > 0) {
            recvBuffer.append(buffer, static_cast<std::size_t>(received));

            std::size_t pos = recvBuffer.find("\r\n");

            while (pos != std::string::npos) {
                if (pos > 510) {
                    disconnectClient(fd);
                    return false;
                }

                std::string line = recvBuffer.substr(0, pos);
                recvBuffer.erase(0, pos + 2);

                if (!line.empty()) {
                    IncomingMessage message;

                    message.fd = fd;
                    message.line = line;

                    _core.onMessage(message);
                }

                pos = recvBuffer.find("\r\n");
            }

        if (recvBuffer.size() > 511 || (recvBuffer.size() == 511 && recvBuffer[510] != '\r')) {
            disconnectClient(fd);
            return false;
        }

        std::cout << "Received " << received << " bytes from fd " << fd << std::endl;

        continue;
    }

        if (received == 0) {
            disconnectClient(fd);
            return false;
        }
        if (errno == EINTR)
            continue;

        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return true;

        disconnectClient(fd);
        return false;
    }
}

bool Server::handleWrite(int fd) {
    std::map<int, Connection>::iterator it = _connections.find(fd);

    if (it == _connections.end())
        throw std::logic_error("handleWrite(): unknown client fd");

    std::string& sendBuffer = it->second.getSendBuffer();

    while (!sendBuffer.empty()) {
        ssize_t sent = send(fd, sendBuffer.data(), sendBuffer.size(), 0);

        if (sent > 0) {
            sendBuffer.erase(0, static_cast<std::size_t>(sent));
            continue;
        }

        if (sent == -1 && errno == EINTR)
            continue;

        if (sent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            return true;
        }

        disconnectClient(fd);
        return false;
    }

    for (std::size_t i = 1; i < _pollFds.size(); i++) {
        if (_pollFds[i].fd == fd) {
            _pollFds[i].events &= ~POLLOUT;
            break;
        }
    }

    return true;
}

void Server::disconnectClient(int fd) {
    if (_connections.find(fd) == _connections.end())
        throw std::logic_error("disconnectClient(): unknown client fd");

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

void Server::run() {
    if (std::signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        throw std::runtime_error("SIGPIPE setup failed");

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

            if (revents & POLLOUT) {
                if (!handleWrite(fd))
                    continue;
            }

            ++i;
        }
    }
}

void Server::queueMessage(const OutgoingMessage& message) {
    std::map<int, Connection>::iterator it = _connections.find(message.fd);

    if (it == _connections.end() || message.data.empty())
        return ;

    for (std::size_t i = 1; i < _pollFds.size(); i++) {
        if (_pollFds[i].fd == message.fd) {
            it->second.getSendBuffer().append(message.data);
            _pollFds[i].events |= POLLOUT;
            return;
        }
    }
    throw std::logic_error("queueMessage(): unknown poll fd");
}
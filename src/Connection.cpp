#include "Connection.hpp"

Connection::Connection(int fd)
    : _fd(fd), _recvBuffer(), _sendBuffer()
{
}

Connection::~Connection()
{
}

int Connection::getFd() const
{
    return _fd;
}

std::string& Connection::getRecvBuffer()
{
    return _recvBuffer;
}

std::string& Connection::getSendBuffer()
{
    return _sendBuffer;
}
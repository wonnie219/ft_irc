#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>

class Connection
{
private:
    int         _fd;
    std::string _recvBuffer;
    std::string _sendBuffer;

public:
    explicit Connection(int fd);
    ~Connection();

    int getFd() const;

    std::string& getRecvBuffer();
    std::string& getSendBuffer();
};

#endif
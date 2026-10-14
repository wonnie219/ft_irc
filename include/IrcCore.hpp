#ifndef IRCCORE_HPP
#define IRCCORE_HPP

#include <string>

#include "Common.hpp"

class IrcCore
{
private:
    std::string _password;

    IrcCore();
    IrcCore(const IrcCore& other);
    IrcCore& operator=(const IrcCore& other);

public:
    explicit IrcCore(const std::string& password);
    ~IrcCore();

    void onMessage(const IncomingMessage& message);
};

#endif

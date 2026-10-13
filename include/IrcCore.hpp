#ifndef IRCCORE_HPP
#define IRCCORE_HPP

#include <string>

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
};

#endif
#include <iostream>

#include "IrcCore.hpp"

IrcCore::IrcCore(const std::string &password)
    : _password(password)
{
}

IrcCore::~IrcCore()
{
}

void IrcCore::onMessage(const IncomingMessage& message)
{
    std::cout << "[IrcCore] fd=" << message.fd << ", line=" << message.line << '\n';
}
// 테스트용
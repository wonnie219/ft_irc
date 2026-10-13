#include "IrcServer.hpp"

IrcServer::IrcServer(int port, const std::string &password)
    : _core(password), _server(port, _core)
{
}

IrcServer::~IrcServer()
{
}

void	IrcServer::run()
{
    _server.run();
}
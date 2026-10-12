#ifndef IRCSERVER_HPP
#define IRCSERVER_HPP

#include <string>

#include <IrcCore.hpp>
#include <Server.hpp>

class IrcServer
{
private:
	IrcCore	_core;
	Server	_server;

	IrcServer();
	IrcServer(const IrcServer &other);
	IrcServer &operator=(const IrcServer &other);
public:
	IrcServer(int port, const std::string &password);
	~IrcServer();

	void	run();
};

#endif
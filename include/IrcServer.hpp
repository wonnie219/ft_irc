#ifndef IRCSERVER_HPP
#define IRCSERVER_HPP

#include <string>

class IrcServer
{
public:
	IrcServer(int port, const std::string &password);
	~IrcServer();

	void	run();
};

#endif
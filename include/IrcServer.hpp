#ifndef IRCSERVER_HPP
#define IRCSERVER_HPP

#include <string>

class IrcServer
{
private:
	IrcServer();
	IrcServer(const IrcServer &other);
	IrcServer &operator=(const IrcServer &other);
public:
	IrcServer(int port, const std::string &password);
	~IrcServer();

	void	run();
};

#endif
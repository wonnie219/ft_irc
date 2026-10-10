#include <iostream>
#include <string>
#include <exception>

#include "IrcServer.hpp"

namespace
{
	int	argCheck(const std::string &portString, const std::string &passString) {
		int		portDigit = 0;

		if (portString.empty()) {
			std::cerr << "Error: Port must be a number" << std::endl;
			return -1;
		}

		if (passString.empty()) {
			std::cerr << "Error: Password cannot be empty" << std::endl;
			return -1;
		}

		for (std::size_t i = 0; i < portString.size(); i++) {
			if (portString[i] < '0' || portString[i] > '9') {
				std::cerr << "Error: Port must be a number" << std::endl;
				return -1;
			}

			portDigit = portDigit * 10 + (portString[i] - '0');

			if (portDigit > 65535) {
				std::cerr << "Error: Port must be between 1 and 65535" << std::endl;
				return -1;
			}
		}
		if (portDigit == 0) {
			std::cerr << "Error: Port must be between 1 and 65535" << std::endl;
			return -1;
		}

		return portDigit;
	}
}

int main(int argc, char** argv) {
    if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <port> <server_password>" << std::endl;
		return 1;
	}

	int port = argCheck(argv[1], argv[2]);

	if (port == -1)
		return 1;

	std::string password = argv[2];

	try {
		IrcServer ircServer(port, password);
		ircServer.run();
	}
	catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
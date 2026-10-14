NAME = ircserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g
INCDIR = include

SRCS =	main.cpp \
		src/IrcServer.cpp\
		src/Server.cpp \
		src/IrcCore.cpp \
		src/Connection.cpp

OBJS = $(SRCS:%.cpp=%.o)

all : $(NAME)

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I $(INCDIR)

$(NAME) : $(OBJS)
	$(CXX) $(OBJS) -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
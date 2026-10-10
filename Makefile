NAME = ircserv

CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98
INCDIR = include

SRCS =	main.cpp \
		src/IrcServer.cpp

OBJS = $(SRCS:%.cpp=%.o)

all : $(NAME)

%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INCDIR)

$(NAME) : $(OBJS)
	$(CC) $(OBJS) -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
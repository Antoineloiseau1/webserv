
NAME		= webserv

SRCS		= testmanon.cpp

OBJS		= ${SRCS:.cpp=.o}

HEADER		= ./includes/

CFLAGS		= -Wall -Wextra -Werror -std=c++98

RM  		= rm -f

CC			= c++

GREEN		:=  "\033[0;32m"

CYAN		:=  "\033[0;36m"

RESET		:=  "\033[0m"

%.o:		%.cpp
				@${CC} ${CFLAGS} -g -o $@ -c $< -I ${HEADER}

${NAME}:	${OBJS}
				@${CC} ${CFLAGS} -o ${NAME} ${OBJS}
				@echo ${GREEN}"Compiled '${NAME}' with success" ${RESET}

all:		${NAME}

clean:
			@${RM} ${OBJS}
			@echo ${CYAN}"Cleaned objects with success"${RESET}

fclean:		clean
			@${RM} ${NAME}
			@echo ${CYAN}"Removed '${NAME}' with success"${RESET}

re:			fclean all

.PHONY:		all clean fclean re                                                                                                                                                   39,1-8        Bot


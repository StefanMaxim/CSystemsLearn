CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wshadow -Wvla --pedantic -Werror
DBG = -g 
OPT = -O2
EXEC = app

SRCS = main.c utils.c mathlib.c
OBJS = $(wildcard $(SRCS):.c=.o)





%.o : %.c
	$(CC) -c $(C)


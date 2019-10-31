TARGETS	=	chat-client	chat-server

CC	=	gcc
CFLAGS	=	-Wall	-Werror	-03

default:	$(TARGETS)

%:	%.c
	$(CC)	$(FLAGS)	-o	$@	$<

clean:
	rm	-f	$(TARGETS)
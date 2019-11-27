TARGETS = file-sender file-receiver
CC = gcc
CFLAGS = -Wall -Werror -O0 -g

default: $(TARGETS)
  
%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
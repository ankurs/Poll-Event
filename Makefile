CC = gcc -fPIC 
LDFLAGS = -lm

# set DEBUG options
ifdef DEBUG
CFLAGS = -Wall -Wextra -ggdb -pg -DDEBUG
else
CFLAGS = -Wall -O2
endif

#name all the object files
OBJS = test.o hash_table/hashtable.o poll.o

all : epoll hash_table

epoll : $(OBJS)
	$(CC) $(LDFLAGS) -o poll $^

hash_table : 
	make -C hash_table

debug :
	make all DEBUG=1

%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $^

doxy :
	doxygen Doxyfile
	sh update_doc.sh	

clean :
	rm -rf $(OBJS) poll doc/ manual.pdf

cs :
	cscope -bRv

cscope :
	cscope -bRv


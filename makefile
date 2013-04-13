CC			= gcc
CFLAGS		= -g -Wall
INCLUDES	= 
LDFLAGS		= 
LIBS		= 
OBJS1		= client.o network.o
OBJS2		= server.o network.o
PROGRAM1	= remote_copy
PROGRAM2	= remote_copy_server

$(PROGRAM1):	$(OBJS1)
				$(CC) $(LDFLAGS) $(LIBS) $(OBJS1) -o $(PROGRAM1)

server:			$(OBJS2)
				$(CC) $(LDFLAGS) $(LIBS) $(OBJS2) -o $(PROGRAM2)

clean:;		rm -f *.o *~ $(PROGRAM1) $(PROGRAM2)

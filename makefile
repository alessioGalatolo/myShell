CC          =  gcc
AR          =  ar
CFLAGS      += -std=c99 -Wall -g
ARFLAGS     =  rvs
INCLUDES	= -I.
LDFLAGS		= -L.
LIBS        = -pthread
OPTFLAGS	= #-O3 


TARGETS		= main
		  
.PHONY: all clean

%: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $< $(LIBS)

%.a: %.o
	$(AR) $(ARFLAGS) lib$@ $<

main: main.c cmd_storage.a cmd_storage.h
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS) $(LIBS) -lcmd_storage -lstack -lRBTree

cmd_storage.a: RBTree.c stack.c 
#	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS) -lobjstr $(LIBS)

cmd_storage.o: cmd_storage.c RBTree.a stack.a
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $< -lRBTree -lstack

all		: $(TARGETS)

clean		: 
	rm -f $(TARGETS)
	rm -f *.o *.a


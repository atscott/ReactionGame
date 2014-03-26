SHELL = /bin/sh
SRCDIR = .
CC = arm-linux-gnueabi-gcc
#CC = gcc
YACC = bison -y
CDEBUG = -g
COMPLIANCE_FLAGS = -save-temps

CFLAGS = $(COMPLIANCE_FLAGS) $(CDEBUG) -I. -I$(SRCDIR)
LDFLAGS = -g
LIBS = -lm -lpthread -lrt

############################################################################################################
# List your sources here.
SOURCES = gpioInterface.c timeutil.c main.c
############################################################################################################

############################################################################################################
# list the name of your output program here.
EXECUTABLE = lab3
############################################################################################################
# Create the names of the object files (each .c file becomes a .o file)
OBJS = $(patsubst %.c, %.o, $(SOURCES))

include $(SOURCES:.c=.d)

all : $(OBJS) $(EXECUTABLE)


install : all
	scp $(EXECUTABLE) root@192.168.0.105:$(EXECUTABLE)


$(EXECUTABLE) : $(OBJS)
	$(CC) -o $(EXECUTABLE)  $(OBJS) $(LIBS)

%.o : %.c #Defines how to translate a single c file into an object file.
	echo compiling $<
	$(CC) $(CFLAGS) -c $<

%.d : %.c #Defines how to generate the dependencies for the given files.  -M gcc option generates dependencies.
	@set -e; rm -f $@; \
	$(CC) $(COMPLIANCE_FLAGS ) -M $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$

clean :   # Delete any and all artifacts from the build.  The only thing which is kept is the source code.
	rm -f *.o
	rm -f *.i
	rm -f *.s
	rm -f *.d
	rm -f $(EXECUTABLE)

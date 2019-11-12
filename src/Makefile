#
# Created by gmakemake (Sparc Sep 12 2019) on Tue Nov 12 11:51:34 2019
#

#
# Definitions
#

.SUFFIXES:
.SUFFIXES:	.a .o .c .C .cpp .s .S
.c.o:
		$(COMPILE.c) $<
.C.o:
		$(COMPILE.cc) $<
.cpp.o:
		$(COMPILE.cc) $<
.S.s:
		$(CPP) -o $*.s $<
.s.o:
		$(COMPILE.cc) $<
.c.a:
		$(COMPILE.c) -o $% $<
		$(AR) $(ARFLAGS) $@ $%
		$(RM) $%
.C.a:
		$(COMPILE.cc) -o $% $<
		$(AR) $(ARFLAGS) $@ $%
		$(RM) $%
.cpp.a:
		$(COMPILE.cc) -o $% $<
		$(AR) $(ARFLAGS) $@ $%
		$(RM) $%

CC =		gcc
CXX =		g++

RM = rm -f
AR = ar
LINK.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)
LINK.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS)
COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) -c
COMPILE.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) -c
CPP = $(CPP) $(CPPFLAGS)
########## Flags from header.mak

LDLIBS = -lncurses -lpthread

# language-specific linker options
CLDLIBS =
CCLDLIBS =

# compiler flags
CCFLAGS = -g $(INCLUDE) -DGL_GLEXT_PROTOTYPES
CFLAGS = -std=c99 -Wall -pthread $(CCFLAGS)
CXXFLAGS = $(CCFLAGS)

# linker flags
LIBFLAGS = -g $(LIBDIRS) $(LDLIBS)
CLIBFLAGS = $(LIBFLAGS) $(CLDLIBS)
CCLIBFLAGS = $(LIBFLAGS) $(CCLDLIBS)

########## End of flags from header.mak


CPP_FILES =	
C_FILES =	client.c client_conn.c generic.c list.c message.c player.c render.c server.c solo_main.c test_board.c test_client_conn.c test_player.c tetris_game.c
PS_FILES =	
S_FILES =	
H_FILES =	client_conn.h generic.h list.h message.h player.h render.h tetris_game.h
SOURCEFILES =	$(H_FILES) $(CPP_FILES) $(C_FILES) $(S_FILES)
.PRECIOUS:	$(SOURCEFILES)
OBJFILES =	client_conn.o generic.o list.o message.o player.o render.o tetris_game.o 

#
# Main targets
#

all:	client server solo_main test_board test_client_conn test_player 

client:	client.o $(OBJFILES)
	$(CC) $(CFLAGS) -o client client.o $(OBJFILES) $(CLIBFLAGS)

server:	server.o $(OBJFILES)
	$(CC) $(CFLAGS) -o server server.o $(OBJFILES) $(CLIBFLAGS)

solo_main:	solo_main.o $(OBJFILES)
	$(CC) $(CFLAGS) -o solo_main solo_main.o $(OBJFILES) $(CLIBFLAGS)

test_board:	test_board.o $(OBJFILES)
	$(CC) $(CFLAGS) -o test_board test_board.o $(OBJFILES) $(CLIBFLAGS)

test_client_conn:	test_client_conn.o $(OBJFILES)
	$(CC) $(CFLAGS) -o test_client_conn test_client_conn.o $(OBJFILES) $(CLIBFLAGS)

test_player:	test_player.o $(OBJFILES)
	$(CC) $(CFLAGS) -o test_player test_player.o $(OBJFILES) $(CLIBFLAGS)

#
# Dependencies
#

client.o:	client_conn.h render.h
client_conn.o:	client_conn.h generic.h message.h player.h render.h tetris_game.h
generic.o:	generic.h
list.o:	list.h
message.o:	generic.h message.h player.h tetris_game.h
player.o:	generic.h list.h player.h tetris_game.h
render.o:	render.h
server.o:	generic.h message.h player.h tetris_game.h
solo_main.o:	tetris_game.h
test_board.o:	render.h
test_client_conn.o:	client_conn.h
test_player.o:	generic.h player.h tetris_game.h
tetris_game.o:	tetris_game.h

#
# Housekeeping
#

Archive:	archive.tgz

archive.tgz:	$(SOURCEFILES) Makefile
	tar cf - $(SOURCEFILES) Makefile | gzip > archive.tgz

clean:
	-/bin/rm $(OBJFILES) client.o server.o solo_main.o test_board.o test_client_conn.o test_player.o core 2> /dev/null

realclean:        clean
	-/bin/rm -rf client server solo_main test_board test_client_conn test_player 

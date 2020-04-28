SOURCES=main.cpp
OBJS=$(patsubst %.cpp, %.o, $(SOURCES))
CPPFLAGS=-std=gnu++11 -pedantic -Wall -Werror
DEPS=Proxy.hpp Socket.hpp Request_Parser.hpp Response_Parser.hpp

proxy: $(OBJS)
	g++ $(CPPFLAGS) -o proxy $(OBJS) -lpthread

%.o: %.cpp $(DEPS)
	g++ $(CPPFLAGS) -c $< -lpthread

clean:
	rm -f proxy *~ *.o

clobber:
	rm -f *~ *.o

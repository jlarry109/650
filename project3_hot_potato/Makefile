all: ringmaster player

ringmaster: ringmaster.cpp server.cpp potato.hpp
	g++ -g -o ringmaster ringmaster.cpp server.cpp 
player: player.cpp server.cpp potato.hpp
	g++ -g -o player player.cpp server.cpp 

.PHONY:
	clean
clean:
	rm -rf *.o ringmaster player
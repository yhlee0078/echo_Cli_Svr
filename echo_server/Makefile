all: echo_server

echo_server: main.o
	g++ -g -w -o echo_server main.o -lpthread

main.o: main.cpp
	g++ -std=gnu++11 -g -c -o main.o main.cpp

clean:
	rm -f echo_server
	rm -f *.o


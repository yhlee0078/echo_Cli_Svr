all: echo_client

echo_client: main.o 
	g++ -g -w -o echo_client main.o -lpthread

main.o:
	g++ -g -w -c -o main.o main.cpp

clean:
	rm -f echo_client
	rm -f *.o

#include <stdio.h> 
#include <string.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <inttypes.h>
#include <string>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;

vector<int> childfd;
mutex m;

void str_to_uint16(char* str, uint16_t* ret) {
    char *end;
    intmax_t val = strtoimax(str, &end, 10);
    *ret = (uint16_t) val;
    return;
}

void connection(int fd, bool broadcast) {
	while (true) {
                const static int BUFSIZE = 1024;
                char buf[BUFSIZE];
                ssize_t received = recv(fd, buf, BUFSIZE - 1, 0);
                if (received == 0 || received == -1) {
                       perror("recv failed");
                       break;
                }
                buf[received] = '\0';
		printf("Client #%d msg: %s\n", fd, buf);
		if(broadcast) {
			bool flg = false;
			for(vector<int>::iterator it = childfd.begin(); it != childfd.end(); it++) {
				ssize_t sent = send(*it, buf, strlen(buf), 0);
				if (sent == 0) {
					perror("broadcast failed");
					flg = true;
					break;
				}
			}
			if(flg) break;
		}
		else {
	                ssize_t sent = send(fd, buf, strlen(buf), 0);
        	        if (sent == 0) {
                	        perror("send failed");
                      		break;
			}
                }
        }
	for(vector<int>::iterator it = childfd.begin(); it != childfd.end();) {
		if(*it == fd) {
			printf("Client #%d disconnected\n", *it);
			m.lock();
			childfd.erase(it);
			m.unlock();
		}
		else 
			it++;
	}
	return;
}


int main(int argc, char* argv[]) {
	bool broadcast = false;
	if(argc < 2 && argc > 3) {
		printf("syntax: echo_server <port> [-b]\n");
		printf("example: echo_server 1234 [-b]\n");
		printf("option [-b]: send message to every clients after receive message\n");
		exit(-1);
	}
	if(argc == 3) {
		if(!strcmp(argv[2], "-b")) {
			broadcast = true;
		}
		else {
			exit(-1);
		}
	}
		
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket failed");
		exit(-1);
	}

	uint16_t port;
	str_to_uint16(argv[1], &port);

	int optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,  &optval , sizeof(int));
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

	int res = bind(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(struct sockaddr));
	if (res == -1) {
		perror("bind failed");
		exit(-1);
	}

	res = listen(sockfd, 2);
	if (res == -1) {
		perror("listen failed");
		exit(-1);
	}

	while (true) {
		struct sockaddr_in addr;
		socklen_t clientlen = sizeof(sockaddr);
		int child = accept(sockfd, reinterpret_cast<struct sockaddr*>(&addr), &clientlen);

		m.lock();	
		childfd.push_back(child);
		m.unlock();

		if (child < 0) {
			perror("ERROR on accept");
			break;
		}
		printf("Client #%d connected\n", child);
		
		if(child > 0) {
			thread t(connection, child, broadcast);
			t.detach();
		}
	}

	close(sockfd);
}

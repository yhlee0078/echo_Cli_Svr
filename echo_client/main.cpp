#include <stdio.h> 
#include <string.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <sys/socket.h> 
#include <inttypes.h>
#include <thread>

using namespace std;

void str_to_uint16(char* str, uint16_t* ret) {
    char *end;
    intmax_t val = strtoimax(str, &end, 10);
    *ret = (uint16_t) val;
    return;
}

void print_IP(uint8_t* ip) {
  printf("%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  return;
}

void parse_IP(char* ip, uint8_t* out) {
  out[0] = (uint8_t)atoi(strtok(ip, "."));
  for(int i = 1; i < 4; i++) {
    out[i] = (uint8_t)atoi(strtok(NULL, "."));
  }
  return;
}

void send_msg(int sockfd) {
	while(true) {
		const static int BUFSIZE = 1024;
                char buf[BUFSIZE];
                scanf("%s", buf);
                if (strcmp(buf, "quit") == 0) break;

                ssize_t sent = send(sockfd, buf, strlen(buf), 0);
                if (sent == 0) {
                        perror("send failed");
                        break;
                }
	}
	return;
}

void recv_msg(int sockfd) {
	while(true) {
		const static int BUFSIZE = 1024;
                char buf[BUFSIZE];
		
		ssize_t received = recv(sockfd, buf, BUFSIZE - 1, 0);
                if (received == 0 || received == -1) {
                        perror("recv failed");
                        break;
                }
                buf[received] = '\0';
                printf("recv msg: %s\n", buf);
	}
	return;
}


int main(int argc, char* argv[]) {
	if(argc != 3) {
		printf("systax: echo_client <host> <port>\n");
		printf("example: echo_client 127.0.0.1 1234\n");
		exit(-1);
	}

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket failed");
		exit(-1);
	}

	uint8_t tmp_ip[4];
	uint16_t port;
	parse_IP(argv[1], tmp_ip);
	str_to_uint16(argv[2], &port);
	printf("Connect to ");
	print_IP(tmp_ip);
	printf(":%d\n", port);
	uint32_t* ip_addr = (uint32_t*)&tmp_ip;
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = *ip_addr;
	memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

	int res = connect(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(struct sockaddr));
	if (res == -1) {
		perror("connect failed");
		exit(-1);
	}
	printf("connected\n");

	thread t1(send_msg, sockfd);
	thread t2(recv_msg, sockfd);

	t1.detach();
	t2.join();

	close(sockfd);
}

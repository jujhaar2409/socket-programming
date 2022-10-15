// code for socket binding taken from https://www.geeksforgeeks.org/socket-programming-cc/ 

// Server side C/C++ program to demonstrate Socket
// programming
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <random>
#include <regex>
#include <chrono>
#define PORT 8080

using namespace std;

// regex pkt_regex("Packet:[0-9]+");
regex pkt_regex("[\\S|\\s]+(Packet:[0-9]+)[\\s|\\S]+");

#define gettime ((std::chrono::system_clock::now() - start).count()) 
#define cout2 cout << gettime << " - "

int seq_num_expect = 1;
string generate_next_acknowledgement(int seq_num)
{
	// seq_num += 1; 
	return "Acknowledgement:" + to_string(seq_num);
}

int get_seq_num(string packet) {
	int index = packet.find(":");
	int seq_num = stoi(packet.substr(index + 1, packet.length() - 1));
	return seq_num;
}

int main(int argc, char const* argv[])
{
	auto start = std::chrono::system_clock::now();
	// for (int i = 0; i < argc; i++) {
	// 	cout2 << argv[i] << endl;
	// }

	int PORT_R = stoi(argv[1]);
	int PORT_S = stoi(argv[2]);
	float DROP_PROB = stof(argv[3]);

	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = { 0 };
	char* hello = "Hello from server";

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	// if (setsockopt(server_fd, SOL_SOCKET,
	// 			SO_REUSEADDR | SO_REUSEPORT, &opt,
	// 			sizeof(opt))) {
	if (setsockopt(server_fd, SOL_SOCKET,
				SO_DEBUG, &opt,
				sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	if ((new_socket
			= accept(server_fd, (struct sockaddr*)&address,
					(socklen_t*)&addrlen))
			< 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}


	while (true) {
		valread = read(new_socket, buffer, 1024);
		cout2 << "recv: " << buffer << endl;
		// if (regex_match(buffer, pkt_regex)) {
		if (buffer[0] == 'P') {
			int seq_num = get_seq_num(buffer);
			cout2 << "seq_num recvd: " << seq_num << endl;
			float r = (float)rand() / (float)RAND_MAX;
			// cout2 << "DROP randnum: " << r << endl;
			bool drop = r < DROP_PROB;
			if (seq_num == seq_num_expect && drop) {
				cout2 << "<< DROP >>" << endl;
				continue;
			} else if (seq_num == seq_num_expect && !drop) {
				cout2 << "<< send ack for NEW packet >>" << endl;
				seq_num_expect += 1;
			} else if (seq_num != seq_num_expect) {
				cout2 << "<< send ack for CURRENT packet >>" << endl;
			}
			string acknowledgement = generate_next_acknowledgement(seq_num_expect);
			send(new_socket, acknowledgement.c_str(), acknowledgement.length(), 0);
			cout2 << "send: " << acknowledgement.c_str() << endl;
		}
		// printf("%s\n", buffer);
		// send(new_socket, hello, strlen(hello), 0);
		// printf("Hello message sent\n");
	}

	// closing the connected socket
	close(new_socket);
	// closing the listening socket
	shutdown(server_fd, SHUT_RDWR);
	return 0;
}

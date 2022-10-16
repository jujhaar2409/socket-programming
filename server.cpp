// code for socket binding taken from https://www.geeksforgeeks.org/socket-programming-cc/ 

#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <random>
// #include <regex>
#include <chrono>

using std::chrono::system_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;
using namespace std;

// regex pkt_regex("[\\S|\\s]+(Packet:[0-9]+)[\\s|\\S]+");

#define gettime duration_cast<milliseconds>(system_clock::now() - start).count()
#define cout2 cout << gettime << " - "

int seq_num_expect = 1;

string generate_next_acknowledgement(int seq_num)
{
	return "Acknowledgement:" + to_string(seq_num);
}

int get_seq_num(string packet) {
	int index = packet.find(":");
	int seq_num = stoi(packet.substr(index + 1, packet.length() - 1));
	return seq_num;
}

int main(int argc, char const* argv[])
{
	auto start = system_clock::now();

	int PORT_R = stoi(argv[1]);
	int PORT_S = stoi(argv[2]);
	float DROP_PROB = stof(argv[3]);

	int server_fd, new_socket;
	struct sockaddr_in serv_address, cli_address;
	char buffer[1024] = { 0 };

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&serv_address, 0, sizeof(serv_address));
	memset(&cli_address, 0, sizeof(cli_address));


	serv_address.sin_family = AF_INET;
	serv_address.sin_addr.s_addr = INADDR_ANY;
	serv_address.sin_port = htons(PORT_R);

	if (bind(server_fd, (struct sockaddr*)&serv_address,
             sizeof(serv_address)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	
	int valread;
	socklen_t length = sizeof(cli_address);
	
	while (true) {
		valread = recvfrom(server_fd, (char *) buffer, 1024, MSG_WAITALL, (struct sockaddr *) &cli_address, &length);
		buffer[valread] ='\0';
		// cout2 << "recv: " << buffer << endl;
		if(buffer[0] == 'P') {
			int seq_num = get_seq_num(buffer);
			// cout2 << "seq_num recvd: " << seq_num << endl;
			float r = (float)rand() / (float)RAND_MAX;
			// cout2 << "DROP randnum: " << r << endl;
			bool drop = r < DROP_PROB;
			if(seq_num == seq_num_expect && drop) {
				cout<<"Dropped Packet"<<endl;
				// cout2 << "<< DROP >>" << endl;
			} 
			else if(seq_num == seq_num_expect && !drop) {
				// cout2 << "<< send ack for NEW packet >>" << endl;
				seq_num_expect += 1;
			} 
			else if (seq_num != seq_num_expect) {
				// cout2 << "<< send ack for CURRENT packet >>" << endl;
			}
			string acknowledgement = generate_next_acknowledgement(seq_num_expect);
			sendto(server_fd, acknowledgement.c_str(), acknowledgement.length(), MSG_CONFIRM, (const struct sockaddr *) &cli_address, length);
			// cout2 << "send: " << acknowledgement.c_str() << endl;
			cout<< acknowledgement.c_str() <<endl;
		}
	}
	shutdown(server_fd, SHUT_RDWR);
	return 0;
}

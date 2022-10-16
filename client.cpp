// code for socket binding taken from https://www.geeksforgeeks.org/socket-programming-cc/ 

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <signal.h>
#include <regex>
#include <chrono>
#include <sys/time.h>
#include <sys/types.h>

using std::chrono::system_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;
using namespace std;

// regex ack_regex("Acknowledgement:[0-9]+");
// regex ack_regex(":*");
regex ack_regex("[\\S|\\s]+(Acknowledgement:[0-9]+)[\\s|\\S]+");

#define gettime duration_cast<milliseconds>(system_clock::now() - start).count()
#define cout2 cout << gettime << " - "

std::chrono::_V2::system_clock::time_point start;

int recvfrom_withtimeout(int s, char * buffer, socklen_t length, sockaddr_in serv_address, int retransmit_time)
{
	fd_set fds;
	int n;
	struct timeval tv;
	
	FD_ZERO(&fds);
	FD_SET(s, &fds);
	
	tv.tv_sec = retransmit_time;
	tv.tv_usec = 0;

	n = select(s+1, &fds, NULL, NULL, &tv);

	if (n == 0){ 
		return -2; // timeout!
	}
	if (n == -1) 
		return -1; // error

	return recvfrom(s, (char *) buffer, 1024, MSG_WAITALL, (struct sockaddr *) &serv_address, &length);
}

int get_seq_num(string packet) {
	int index = packet.find(":");
	int seq_num = stoi(packet.substr(index + 1, packet.length() - 1));
	return seq_num;
}

int main(int argc, char const* argv[])
{

	struct sockaddr_in serv_address;
	int seq_num_send;
	int PORT_S;
	int PORT_R;
	int RETRANSMIT_TIME;
	int NUM_PACKETS;

	bool got_ack = true;
	int client_fd;
	
	start = std::chrono::system_clock::now();

	PORT_S = stoi(argv[1]);
	PORT_R = stoi(argv[2]);
	RETRANSMIT_TIME = stof(argv[3])*1000;
	NUM_PACKETS = stoi(argv[4]);

	//generate packets to transmit
	vector<string> packets;
	
	for(int i = 1; i <= NUM_PACKETS; i++){
		string pkt = "Packet:" + to_string(i);
		packets.push_back(pkt);
	}
	
	int valread;
    char buffer[1024] = { 0 };
    if ((client_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("\n socket creation failure \n");
        return -1;
    }
	
	memset(&serv_address, 0, sizeof(serv_address));

    serv_address.sin_family = AF_INET;
    serv_address.sin_port = htons(PORT_R);
	serv_address.sin_addr.s_addr = INADDR_ANY;

	socklen_t length;
	
	seq_num_send = 1;
	while(seq_num_send <= NUM_PACKETS){
		string packet = packets[seq_num_send - 1];
		sendto(client_fd, packet.c_str(), packet.length(), MSG_CONFIRM, (const struct sockaddr *) &serv_address, sizeof(serv_address)); 
		cout2 << "send: " << packet.c_str() << endl;
		valread = recvfrom_withtimeout(client_fd, buffer, length, serv_address, length);
		if(valread > 0){
			buffer[valread] = '\0';
			cout2 << "recv: " << buffer << endl;
			// if(regex_match(buffer, ack_regex)) {
			if(buffer[0] == 'A') {
				cout2 << "regex match" << endl;
				int seq_num = get_seq_num(buffer);
				// cout2 << "seq"
				cout2 << "seq_num recvd: " << seq_num << endl;
				if(seq_num == seq_num_send + 1){
					seq_num_send += 1;
				}
			}
		}
	}
	// closing the connected socket
	close(client_fd);
	return 0;
}

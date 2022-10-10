// code for socket binding taken from https://www.geeksforgeeks.org/socket-programming-cc/ 

// Client side C/C++ program to demonstrate Socket
// programming
#include <arpa/inet.h>
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
#define PORT 8080

using namespace std;
regex ack_regex("Acknowledgement:[0-9]+");

int seq_num_send;
int PORT_S;
int PORT_R;
int RETRANSMIT_TIME;
int NUM_PACKETS;
bool got_ack = true;
string packet;
int sock;

//interrupt service routine
void timeout(int sig)
{
	send(sock, packet.c_str(), packet.length(), 0); 
	signal(SIGALRM, &timeout);  // set a signal handler
	ualarm(RETRANSMIT_TIME * 1e3, 0);
}

int get_seq_num(string packet) {
	int index = packet.find(":");
	int seq_num = stoi(packet.substr(index + 1, packet.length() - 1));
	return seq_num;
}

int main(int argc, char const* argv[])
{
	PORT_S = stoi(argv[1]);
	PORT_R = stoi(argv[2]);
	RETRANSMIT_TIME = stoi(argv[3]);
	NUM_PACKETS = stoi(argv[4]);

	//generate packets to transmit
	vector<string> packets;
	for(int i = 1; i <= NUM_PACKETS; i++){
		string pkt = "Packet:" + to_string(i);
		packets.push_back(pkt);
	}
	
	sock = 0; 
	int valread, client_fd;
    struct sockaddr_in serv_addr;
    char buffer[1024] = { 0 };
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
 
    if((client_fd = connect(sock, (struct sockaddr*)&serv_addr,sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

	seq_num_send = 1;
	while(seq_num_send <= NUM_PACKETS){
		packet = packets[seq_num_send - 1];
		if(got_ack){	
			send(sock, packet.c_str(), packet.length(), 0); 
			signal(SIGALRM, &timeout);  // set a signal handler
			ualarm(RETRANSMIT_TIME * 1e3, 0);
		}
		valread = read(sock, buffer, 1024);
		if(regex_match(buffer, ack_regex)) {
			int seq_num = get_seq_num(buffer);
			if(seq_num == seq_num_send + 1){
				seq_num_send += 1;
				printf("%s\n", buffer);
				got_ack = true;
			}
			else{
				got_ack = false;
			}
			continue;
		}
	}
	// closing the connected socket
	close(client_fd);
	return 0;
}

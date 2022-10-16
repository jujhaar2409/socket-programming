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

struct sockaddr_in serv_address;
int seq_num_send;
int PORT_S;
int PORT_R;
int RETRANSMIT_TIME;
int NUM_PACKETS;
bool got_ack = true;
string packet;
int client_fd;
std::chrono::_V2::system_clock::time_point start;

//interrupt service routine
void timeout(int sig)
{
	sendto(client_fd, packet.c_str(), packet.length(), MSG_CONFIRM, (const struct sockaddr *) &serv_address, sizeof(serv_address)); 
	cout2 << "isr send: " << packet.c_str() << endl;
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
		packet = packets[seq_num_send - 1];
		if(got_ack){	
			sendto(client_fd, packet.c_str(), packet.length(), MSG_CONFIRM, (const struct sockaddr *) &serv_address, sizeof(serv_address)); 
			cout2 << "send: " << packet.c_str() << endl;
			signal(SIGALRM, &timeout);  // set a signal handler
			ualarm(RETRANSMIT_TIME * 1e3, 0);
		}
		valread = recvfrom(client_fd, (char *) buffer, 1024, MSG_WAITALL, (struct sockaddr *) &serv_address, &length);
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
				// printf("%s\n", buffer);
				// cout2 << buffer << endl;
				got_ack = true;
			}
			else{
				got_ack = false;
			}
			continue;
		}
		else{
			got_ack = false;
		}
	}
	// closing the connected socket
	close(client_fd);
	return 0;
}

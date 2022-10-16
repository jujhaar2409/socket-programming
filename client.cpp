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
// #include <signal.h>
// #include <regex>
#include <chrono>
#include <sys/time.h>
#include <sys/types.h>

using std::chrono::system_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::microseconds;
using namespace std;

// regex ack_regex("Acknowledgement:[0-9]+");
// regex ack_regex(":*");
// regex ack_regex("[\\S|\\s]+(Acknowledgement:[0-9]+)[\\s|\\S]+");

// #define gettime duration_cast<microseconds>(system_clock::now() - start).count()
// #define cout2 cout << gettime << " - "

std::chrono::_V2::system_clock::time_point start;
int seq_num_send;


int get_seq_num(string packet) {
	int index = packet.find(":");
	int seq_num = stoi(packet.substr(index + 1, packet.length() - 1));
	return seq_num;
}


void recvfrom_withtimeout(int s, char * buffer, socklen_t length, sockaddr_in serv_address, int retransmit_time)
{
	start = std::chrono::system_clock::now();
	
	fd_set fds;
	int n;
	struct timeval tv;
	
	FD_ZERO(&fds);
	FD_SET(s, &fds);
	
	tv.tv_sec = 0;
	tv.tv_usec = retransmit_time*1000*1000;

	int curr_time = duration_cast<microseconds>(system_clock::now() - start).count();
	// cout<<curr_time<<" "<<retransmit_time*1000*1000<<endl;
	while(curr_time < retransmit_time*1000*1000){
		n = select(s+1, &fds, NULL, NULL, &tv);
		if(n > 0){
			int x = recvfrom(s, (char *) buffer, 1024, MSG_WAITALL, (struct sockaddr *) &serv_address, &length);
			buffer[x] = '\0';
			// cout2 << "recv: " << buffer << endl;
			// if(regex_match(buffer, ack_regex)) {
			if(buffer[0] == 'A') {
				// cout2 << "regex match" << endl;
				int seq_num = get_seq_num(buffer);
				// cout2 << "seq"
				// cout2 << "seq_num recvd: " << seq_num << endl;
				if(seq_num == seq_num_send + 1){
					seq_num_send += 1;
					return;
				}
				else{					 
					curr_time = duration_cast<microseconds>(system_clock::now() - start).count();
					if(retransmit_time*1000*1000 < curr_time){
						cout<<"Retransmission Time Expired!"<<endl;
						return;
					}
					tv = timeval();
					tv.tv_sec = 0;
					tv.tv_usec = retransmit_time*1000*1000 - curr_time;
					// cout<<curr_time<<" "<<retransmit_time*1000*1000<<endl;
				}
			}
		}
		else if(n == 0){
			cout<<"Retransmission Time Expired!"<<endl;
			return;
		}
	}
	cout<<"Retransmission Time Expired!"<<endl;
	return;
}

int main(int argc, char const* argv[])
{

	struct sockaddr_in serv_address;
	int PORT_S;
	int PORT_R;
	int RETRANSMIT_TIME;
	int NUM_PACKETS;

	bool got_ack = true;
	int client_fd;

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
		cout<<packet.c_str()<<endl;
		// cout2 << "send: " << packet.c_str() << endl;
		recvfrom_withtimeout(client_fd, buffer, length, serv_address, RETRANSMIT_TIME);
	}
	// closing the connected socket
	close(client_fd);
	return 0;
}

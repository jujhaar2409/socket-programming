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

/*
https://www.boost.org/doc/libs/1_44_0/doc/html/boost_asio/tutorial/tuttimer2.html
*/

#include <regex>

#define PORT 8080

using namespace std;
regex ack_regex("Acknowledgement:[0-9]+");
int seq_num_send = 1;
vector<string> packets;

int PORT_S;
int PORT_R;
int RETRANSMIT_TIME;
int NUM_PACKETS;

void timeout(int sig)
{

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

	for(int i = 1; i <= NUM_PACKETS; i++){
		string pkt = "Packet:" + to_string(i);
		packets.push_back(pkt);
	}

	int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;
    char buffer[1024] = { 0 };
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
	// serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
 
    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    // if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
    //     <= 0) {
    //     printf(
    //         "\nInvalid address/ Address not supported \n");
    //     return -1;
    // }
 
    if ((client_fd
         = connect(sock, (struct sockaddr*)&serv_addr,
                   sizeof(serv_addr)))
        < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

	while(seq_num_send <= NUM_PACKETS){
		string packet = packets[seq_num_send - 1];
		send(sock, packet.c_str(), packet.length(), 0);

		signal(SIGALRM, &timeout);  // set a signal handler
		ualarm(RETRANSMIT_TIME * 1e3, 0);  // set an alarm for 10 seconds from now

		valread = read(sock, buffer, 1024);
		if(regex_match(buffer, ack_regex)) {
			int seq_num = get_seq_num(buffer);
			
		}
		valread = read(sock, buffer, 1024);
		printf("%s\n", buffer);
	}

	// for (int i = 0; i < 5; i++) {
	// 	string msg = "index: " + to_string(i);
	// 	const char *msg2 = msg.c_str();
	// 	send(sock, msg2, strlen(msg2), 0);
	// 	valread = read(sock, buffer, 1024);
	// 	printf("%s\n", buffer);
	// }

	// closing the connected socket
	close(client_fd);
	return 0;
}

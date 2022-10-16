// socket binding and udp code taken from https://www.geeksforgeeks.org/udp-server-client-implementation-c/

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
#include <chrono>

using std::chrono::system_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;
using namespace std;

int seq_num_expect = 1; //variable to hold expected sequence number

string generate_next_acknowledgement(int seq_num)
{
	//function to generate the next acknowledgement
	return "Acknowledgement:" + to_string(seq_num);
}

int get_seq_num(string packet) {
	// function to get the sequence number from packet
	int index = packet.find(":");
	int seq_num = stoi(packet.substr(index + 1, packet.length() - 1));
	return seq_num;
}

int main(int argc, char const* argv[])
{
	int PORT_R = stoi(argv[1]);
	int PORT_S = stoi(argv[2]);
	float DROP_PROB = stof(argv[3]);

	//code taken from gfg starts here
	int server_fd, new_socket;
	struct sockaddr_in serv_address, cli_address;
	char buffer[1024] = { 0 };

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

	//code taken from gfg ends here
	
	int valread;
	socklen_t length = sizeof(cli_address);
	
	while (true) { //receiver listens indefinitely for packets
		//listen for packets from sender
		valread = recvfrom(server_fd, (char *) buffer, 1024, MSG_WAITALL, (struct sockaddr *) &cli_address, &length);
		buffer[valread] ='\0';
		int seq_num = get_seq_num(buffer); //obtain sequence number from packet

		float r = (float)rand() / (float)RAND_MAX; //obtain a random number from 0 to 1

		bool drop = r < DROP_PROB; //if r < drop_prob drop the packet
		if(drop) {
			//if the packet has been dropped print to the terminal
			cout<<"Dropped Packet"<<endl;
		} 
		else if(seq_num == seq_num_expect) {
			//if the packet is not dropped and expected then increment expected seq number
			seq_num_expect += 1;
		} 
		//if neither of the above indicates a wrong packet
		string acknowledgement = generate_next_acknowledgement(seq_num_expect); //generate the next acknowledgement
		sendto(server_fd, acknowledgement.c_str(), acknowledgement.length(), MSG_CONFIRM, (const struct sockaddr *) &cli_address, length);
		//send the new acknowledgement
		cout<< acknowledgement.c_str() <<endl;
	}
	return 0;
}

// socket binding and udp code taken from https://www.geeksforgeeks.org/udp-server-client-implementation-c/
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

#include <chrono>
#include <sys/time.h>
#include <sys/types.h>

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::system_clock;
using namespace std;

std::chrono::_V2::system_clock::time_point start;
int seq_num_send; // function to hold the current value of sequence number to be sent

int get_seq_num(string packet)
{
	// function to get the sequence number from packet
	int index = packet.find(":");
	int seq_num = stoi(packet.substr(index + 1, packet.length() - 1));
	return seq_num;
}

void recvfrom_withtimeout(int s, char *buffer, socklen_t length, sockaddr_in serv_address, int retransmit_time)
{
	// function to handle timeouts
	// reference : Page #60 Beej's Guide To Socket Programming - implementing timeout using select()

	// store start time at function start
	start = std::chrono::system_clock::now();

	fd_set fds;
	int n;
	struct timeval tv;

	FD_ZERO(&fds);
	FD_SET(s, &fds);

	tv.tv_sec = 0;
	tv.tv_usec = retransmit_time * 1000 * 1000; // the retransmission time is initially set to retransmit_time*1e6 in us

	// calculate time elapsed from start
	int curr_time = duration_cast<microseconds>(system_clock::now() - start).count();

	while (curr_time < retransmit_time * 1000 * 1000)
	{
		// check if the timeout occured
		n = select(s + 1, &fds, NULL, NULL, &tv);
		// perform a select() timeout for time of tv
		if (n > 0)
		{
			// response has been obtained in tv time
			int x = recvfrom(s, (char *)buffer, 1024, MSG_WAITALL, (struct sockaddr *)&serv_address, &length);
			buffer[x] = '\0';
			int seq_num = get_seq_num(buffer);
			// check if seq_num of acknowledgement is equal to prev val sent + 1
			if (seq_num == seq_num_send + 1)
			{
				seq_num_send += 1;
				// if so increment and proceed to send next packet
				return;
			}
			else
			{
				// erroneous acknowledgement has been sent
				curr_time = duration_cast<microseconds>(system_clock::now() - start).count();
				// find new elapsed time
				if (retransmit_time * 1000 * 1000 < curr_time)
				{
					// check if the timeout occured
					cout << "Retransmission Time Expired!" << endl;
					return;
				}
				// create new timeval instance with timer of retransmit time - elasped time
				tv = timeval();
				tv.tv_sec = 0;
				tv.tv_usec = retransmit_time * 1000 * 1000 - curr_time;
				// proceed to perform select() through loop
			}
		}
		else if (n == 0)
		{
			//no response has been obtained in retransmit time so timeout
			cout << "Retransmission Time Expired!" << endl;
			return;
		}
	}
	// if broke out of while then timeout happened
	cout << "Retransmission Time Expired!" << endl;
	return;
}

int main(int argc, char const *argv[])
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

	// generate packets to transmit
	vector<string> packets;

	for (int i = 1; i <= NUM_PACKETS; i++)
	{
		string pkt = "Packet:" + to_string(i);
		packets.push_back(pkt);
	}
	//code taken from gfg starts here
	char buffer[1024] = {0};
	if ((client_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("\n socket creation failure \n");
		return -1;
	}

	memset(&serv_address, 0, sizeof(serv_address));

	serv_address.sin_family = AF_INET;
	serv_address.sin_port = htons(PORT_R);
	serv_address.sin_addr.s_addr = INADDR_ANY;
	//code taken from gfg ends here

	socklen_t length;
	seq_num_send = 1; //initialize x with 1

	while (seq_num_send <= NUM_PACKETS) //check if all packets are sent
	{
		string packet = packets[seq_num_send - 1];
		//send the packet
		sendto(client_fd, packet.c_str(), packet.length(), MSG_CONFIRM, (const struct sockaddr *)&serv_address, sizeof(serv_address));
		cout << packet.c_str() << endl;
		//perform timeout check
		recvfrom_withtimeout(client_fd, buffer, length, serv_address, RETRANSMIT_TIME);
	}
	// closing the connected socket
	close(client_fd);
	return 0;
}

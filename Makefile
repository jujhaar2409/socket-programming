

client : 
	g++ -o client client.cpp

server : 
	g++ -o server server.cpp

all : client server

clean : 
	rm -f client server

fresh : 
	make clean 
	make all

# sender.c <SenderPort> <ReceiverPort> <RetransmissionTimer> <NoOfPacketsToBeSent>
sendr :
	./client 8080 8080 100 10

# receiver.c <ReceiverPort> <SenderPort> <PacketDropProbability>
recvr :
	./server 8080 8080 0.1
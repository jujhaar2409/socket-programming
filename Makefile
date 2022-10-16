

r : 
	g++ -o receiver receiver.cpp

s : 
	g++ -o sender sender.cpp

all : r s

clean : 
	rm -f r s

fresh : 
	make clean 
	make all

# sender.c <SenderPort> <ReceiverPort> <RetransmissionTimer(s)> <NoOfPacketsToBeSent>
se :
	./sender 8080 8082 2 10 > sender.txt

# receiver.c <ReceiverPort> <SenderPort> <PacketDropProbability>
re :
	./receiver 8082 8080 0.3 > receiver.txt
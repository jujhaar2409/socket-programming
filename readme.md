# Implementation of Stop-and-Wait-Algorithm

## Course Assignment - CS 224 - Computer Networks

The problem statement can be found [here](https://github.com/rohankalbag/cs224-iitb/blob/main/Problem%20Statements/CS224M_HW3.pdf)

Explanation for the code, References can be found as inline comments in the code. In brief the code creates UDP sockets for the sender and reciever and they communicate using Stop-and-Wait algorithm as per the problem statement

### System Requirements

- Ubuntu `20.04`
- `g++` version `9.4.0`

### Steps to compile the Code

- Open a terminal in the same directory

- Enter the following in terminal one by one

```bash
g++ -o receiver reciever.cpp
g++ -o sender.cpp
```

### Steps to execute the Code

- Network Delay can be simulated using the following command

  > ```bash
  > sudo tc qdisc add dev lo root netem delay DelayInMilliseconds
  > ```
  >
- Open Two Different Terminals T1 and T2
- In T1 enter

  > ```bash
  > ./sender SenderPort ReceiverPort RetransmissionTimer(s) NoOfPacketsToBeSent
  > ```
  >
- In T2 enter

  > ```bash
  > ./receiver ReceiverPort SenderPort PacketDropProbability
  > ```
  >

- After `sender.cpp` has done executing manually terminate `receiver.cpp` by `Ctrl + C` (this will not affect receiver.txt) in T2



The `sender.txt`, `receiver.txt` attached are simulated for the following commands

```bash
sudo tc qdisc add dev lo root netem delay 1000
./sender 8080 8082 2 10 > sender.txt
./receiver 8082 8080 0.3 > receiver.txt
```

Collaborators

- Jujhaar Singh
- Rohan Rajesh Kalbag

# TCP Handshake Client using Raw Sockets

This C++ program implements the client side of the TCP three-way handshake using raw sockets. It constructs and sends a custom SYN packet to a given server, receives a SYN-ACK, and responds with a final ACK to complete the connection. The client operates at the IP level, manually crafting headers and computing TCP checksums, providing low-level control over the handshake process.

## Features

- Raw Socket Usage: Bypasses the operating system’s TCP stack to send and receive raw IP packets containing TCP segments.
- Custom TCP Handshake:
  - Sends SYN packet with manually set sequence number.
  - Receives SYN-ACK from server and verifies expected sequence and acknowledgment numbers.
  - Sends final ACK to complete handshake.
- Checksum Calculation: Manually computes TCP checksum using a pseudo-header as per the TCP/IP specification.
- Timeout Handling: Implements a receive timeout to avoid hanging if the server doesn't respond.

## Prerequisites

This code requires:

- A Linux system with support for raw sockets.
- A C++ compiler (g++ recommended).
- Root privileges to execute the program due to the use of raw sockets.

## How to Run the Code

### 1. Compile the Program

g++ client.cpp -o client

### 2. Start the Server

Make sure to first clone the course repository and start the provided server:

git clone https://github.com/privacy-iitk/cs425-2025.git
cd cs425-2025/Homeworks/A3
g++ server.cpp -o server
sudo ./server

> Note: You must run the server using sudo since it also uses raw sockets.

### 3. Run the Client

In the same or a different terminal:

sudo ./client

Root access is necessary to create raw sockets.

## How It Works

1. The client creates a raw TCP socket and manually enables the IP_HDRINCL option so it can supply its own IP header.
2. It crafts a TCP SYN packet with a fixed sequence number (e.g., 200) and sends it to the server.
3. It waits to receive a SYN-ACK response from the server. The expected sequence number from the server is fixed (e.g., 400) as per the assignment constraints.
4. After receiving the correct SYN-ACK, it sends a final ACK packet with sequence number 600 and the appropriate acknowledgment number.
5. Upon successful transmission of the ACK, the handshake is considered complete.

## Output

Example output if the handshake completes successfully:

Sending SYN (seq=200)
Received SYN-ACK (seq=400 ack=201)
Sending ACK (seq=600)
Handshake complete

If the handshake fails (e.g., timeout or invalid packet):

SYN-ACK not received

## Notes

- Sequence numbers are hardcoded (200, 400, 600) to match what the server expects.
- The code sets a 5-second timeout for receiving the SYN-ACK packet to avoid indefinite blocking.
- This client assumes both client and server are running on 127.0.0.1 (localhost).


## File Description

- client.cpp: Contains the entire implementation of the TCP client with detailed comments.
- README.md: This file, explaining how to compile, run, and understand the code.

## Contributors and Work Division

All members contributed equally to this assignment.

- Rohan (220741)  
  
- Havish (220879)  
  
- Vijay (220602)  
 

#include <stdio.h>

// Socket libraries
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

// time 
#include <time.h>

#define BUFSIZE 215 // Size of message buffer from client
#define PORT 1234
#define IP "127.0.0.1"

int main(int argc, char* argv[]) { 
    int sd;                            // Socket descriptor 
    char buf[BUFSIZE] = {0};           // Client Message buffer
    struct sockaddr_in server, client; // sockaddr structs for server and client 
    int client_len = sizeof(client);

    /** Initialize udp socket to start listening for client */
    sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sd < 0) { 
        printf("Error creating UDP socket\n");
        return -1;
    }
    printf("Socket created --> %d\n", sd);

    // Set server parameters
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(IP);

    // Bind 
    if (bind(sd, (struct sockaddr*)&server, sizeof(server)) < 0) { 
        printf("Unable to bind to port\n");
        return -1;
    }
    printf("Binded to %s:%d\n", IP, PORT);

    int r;
    int num = 0; // Number of messages received
    // Loop to receive messages from clients

    printf("Listening...\n");
    while (1) { 
        /** Received message from client, ... */
        r = recvfrom(sd, buf, BUFSIZE, 0, (struct sockaddr*)&client, &client_len);
        if  (r < 0) { 
            printf("Bad receive\n");
            continue;
        }
        printf("Message #%d\n%lu: IP:%s\n", ++num, (unsigned long)time(NULL), inet_ntoa(client.sin_addr));

        /** Get/decrypt message */
        printf("Message: %s\n", buf);
    }

    close(sd); // Close socket

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Socket libraries
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define NUM_ARGS 3
#define BUFSIZE 250

enum Args{None, Ip, Port};

/**
 * usage
 * 
 * Prints the usage of the program
*/
void usage() { 
	char* usage_string = "Usage: client [server_ip] [server_port]\n" 
						 "UDP client \n\n" 
                         "-h      Show this information\n";
    
	printf("%s", usage_string);
    exit(0);
}

int main(int argc, char *argv[]) { 
    int sd; // Socket descriptor
    struct sockaddr_in server;
    char buf[BUFSIZE] = {0};
    int server_len = sizeof(server);

    char *ip; // Ip address of server
    int port; // Port to connect to server

    // Read positional arguments for ip and port from command line
    if (argc != NUM_ARGS) { usage(); }
    ip = argv[Ip];
    port = atoi(argv[Port]);

    // Create socket
    sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sd < 0) { 
        printf("Error creating UDP socket\n");
        return -1;
    }
    printf("Socket created --> %d\n", sd);

    // Set server parameters
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    // User input loop and send
    while(1) { 
        printf("Message: ");
        fgets(buf, sizeof(buf), stdin); // Store user input into buffer

        // Send message to server
        if (sendto(sd, buf, strlen(buf), 0, (struct sockaddr*)&server, server_len) < 0) { 
            printf("Error sending message\n");
        }
    }

    close(sd); // Close socket

    return 0;
}
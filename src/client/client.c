#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Socket libraries
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <net/if.h>

// RSA encryption libraries
#include "rsa_enc.h"
#include "rsa_dec.h"

#define NUM_ARGS 3
#define BUFSIZE 250

// RSA defines
#define PUBKEY "public_key.pem"
#define PRVKEY "private_key.pem"
#define DECRYPT_LEN 128

//#define IPV6 1 // Defines whether to use ipv4 or ipv6

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
#ifdef IPV6
    struct sockaddr_in6 server, client;
#else
    struct sockaddr_in server;
#endif
    unsigned char buf[BUFSIZE] = {0};
    unsigned char enc_buf[BUFSIZE] = {0}; // Buffer for holding encrypted data
    int server_len = sizeof(server);

    char *ip; // Ip address of server
    int port; // Port to connect to server

    // Read positional arguments for ip and port from command line
    if (argc != NUM_ARGS) { usage(); }
    ip = argv[Ip];
    port = atoi(argv[Port]);

    // Get hostname of client
    char hostname[1024] = {0}; 
    gethostname(hostname, sizeof(hostname));

#ifdef IPV6 
    sd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    
    if (sd < 0) { 
        printf("Error creating UDP socket\n");
        return -1;
    }
    printf("Socket created --> %d\n", sd);

    server.sin6_family = AF_INET6;
    server.sin6_port = htons(port);
    inet_pton(AF_INET6, ip, &server.sin6_addr); // set ipv6 address
    server.sin6_scope_id = if_nametoindex("enp7s0"); 
    connect(sd, (struct sockaddr *)&server, sizeof(server));
#else
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
#endif

    int enc_len = 0;          // If nonzero, will hold length of encrypted message in buffer
    int success = 0;
    // User input loop and send
    while(1) { 
        printf("%s: ", hostname);
        fgets(buf, sizeof(buf), stdin); // Store user input into buffer

        // Encrypt message and send to server
        enc_len = rsa_enc(buf, strlen(buf), PUBKEY, enc_buf);

        if (enc_len == 0) { 
            printf("Failed to encrypt message\n");
            memset(buf, 0, sizeof(buf));
            memset(enc_buf, 0, sizeof(enc_buf));
            continue;
        }    

#ifdef DEBUG
        // Print encrypted text 
        printf("Encrypted message (in hexadecimal):\n");
        for (int i = 0; i < enc_len; i++) {
            printf("%02x", enc_buf[i]);
        }
        printf("\n");
        printf("Len: %ld\n", strlen(enc_buf));
#endif

        // Send message to server
        int count = sendto(sd, enc_buf, enc_len, 0, (struct sockaddr*)&server, server_len);
        if (count < 0) { 
            printf("Error sending message\n");
            memset(buf, 0, sizeof(buf));
            memset(enc_buf, 0, sizeof(enc_buf));
            continue;
        }

        system("clear"); // Clear terminal screen

        // Listen for last 5 messages from server
        for (int i = 0; i < 5; i++) { 
            if  (recvfrom(sd, buf, BUFSIZE, 0, (struct sockaddr*)&server, &server_len) < 0) { 
                continue; //printf("Error receiving list of messages\n");
            }

            if (!strlen(buf)) { 
                continue;
            }

            success = rsa_dec(buf, DECRYPT_LEN, PRVKEY);
            printf("%s\n", buf);
            memset(buf, 0, sizeof(buf)); // Reset buffer
        }

        memset(buf, 0, sizeof(buf));
        memset(enc_buf, 0, sizeof(enc_buf));
    }

    close(sd); // Close socket

    return 0;
}
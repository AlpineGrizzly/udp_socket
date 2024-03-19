#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Socket libraries
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <net/if.h>

// time 
#include <time.h>

// rsa libraries
#include "rsa_enc.h"
#include "rsa_dec.h"

// RSA defines
#define PUBKEY "public_key.pem"
#define PRVKEY "private_key.pem"
#define DECRYPT_LEN 128

//#define IPV6 1 // Defines whether to use ipv4 or ipv6
               
#define BUFSIZE 250 // Size of message buffer from client
#define PORT 1234

#ifdef IPV6 
#define IP "::1" //"127.0.0.1"
#else 
#define IP "127.0.0.1"
#endif

#define LISTSIZE 5 // Number of recent messages to save

int main(int argc, char* argv[]) { 
    int sd;                            // Socket descriptor 
    char buf[BUFSIZE] = {0};           // Client Message buffer
#ifdef IPV6 
    struct sockaddr_in6 server, client;
#else 
    struct sockaddr_in server, client; // sockaddr structs for server and client 
#endif
    int client_len = sizeof(client);

    /** Initialize udp socket to start listening for client */
#ifdef IPV6
    sd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    
    if (sd < 0) { 
        printf("Error creating UDP socket\n");
        return -1;
    }
    printf("Socket created --> %d\n", sd);

    server.sin6_family = AF_INET6;
    server.sin6_port = htons(PORT);
    server.sin6_addr = in6addr_any;
    server.sin6_scope_id = if_nametoindex("enp7s0"); 
    
#else 
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
#endif

    // Bind 
    if (bind(sd, (struct sockaddr*)&server, sizeof(server)) < 0) { 
        printf("Unable to bind to port\n");
        return -1;
    }
    printf("Binded to %s:%d\n", IP, PORT);

    int r;
    int num = 0;                  // Number of messages received
    char message[BUFSIZE];        // Array to hold message

    char list[LISTSIZE][BUFSIZE] = {0}; // Array to hold last 5 messages received 
    time_t now;                   // Holds current local time
    struct tm *tinfo;             // struct to hold time information
    char *time_str;               // String to hold time information
    int success = 0;              // Indicates whether enc/dec was successful

    printf("Start listening...\n");
    while (1) { 
        /** Received message from client, ... */
        r = recvfrom(sd, buf, BUFSIZE, 0, (struct sockaddr*)&client, &client_len);
        if  (r < 0) { 
            printf("Bad receive\n");
            continue;
        }

        // Get ascii time
        time(&now);
        tinfo = localtime(&now);
        time_str = asctime(tinfo);
        time_str[strcspn(time_str, "\n")] = 0; // Remove trailing newline
        
        //system("clear");

        // Print encrypted received text 
#ifdef DEBUG
        printf("Encrypyed message (in hexadecimal):\n");
        for (int i = 0; i < DECRYPT_LEN; i++) {
            printf("%02x", buf[i]);
        }
        printf("\n");
#endif
        /** Get/decrypt/display message */
        success = rsa_dec(buf, DECRYPT_LEN, PRVKEY);
        system("clear"); // clear terminal screen
        
#ifdef DEBUG
        printf("Len %d\n", DECRYPT_LEN);
        
        // Print decrypted text 
        printf("Decrypted message (in hexadecimal):\n");
        for (int i = 0; i < DECRYPT_LEN; i++) {
            printf("%02x", buf[i]);
        }
        printf("\n");
#endif
    
#ifdef IPV6
        char str2ipv6[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &client.sin6_addr, str2ipv6, INET6_ADDRSTRLEN);
        sprintf(message, "Message #%d\n%s: IP:%s\n", num+1, time_str, str2ipv6);
#else
        sprintf(message, "Message #%d\n%s: IP:%s\n", num+1, time_str, inet_ntoa(client.sin_addr));
#endif
        strcat(message, buf); // Concatenate message to header

        /* Put message in last 5 list */
        if (strcmp(buf, "\n") != 0) { // Don't update list for line feed
            if (num < LISTSIZE) { 
                // Simply append at index num
                strcpy(list[num], message);
            } else { 
                // Shift all message back by one and append to end
                for (int i = 0; i < LISTSIZE-1; i++) { 
                    strcpy(list[i], list[i+1]);
                }
                strcpy(list[LISTSIZE-1], message); // Append new message to end
            }
            num++;
        }

        int enc_len; 
        unsigned char enc_buf[BUFSIZE] = {0};
        memset(buf, 0, sizeof(buf));

        for (int j = 0; j < LISTSIZE; j++) { 
            printf("%s\n", list[j]); // Print to terminal screen
            
            // Encrypt and send payload to client
            enc_len = rsa_enc(list[j], strlen(list[j]), PUBKEY, enc_buf);

            // if enc_len == 0 failed

            if (sendto(sd, enc_buf, enc_len, 0, (struct sockaddr*)&client, client_len) < 0) { 
                printf("Error sending message\n");
                continue;
            }

        }

        /* Wipe buffer squeaky clean */
        memset(buf, 0, sizeof(buf));
    }

    close(sd); // Close socket

    return 0;
}

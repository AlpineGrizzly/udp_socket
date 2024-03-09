#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Socket libraries
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

// time 
#include <time.h>

#define BUFSIZE 250 // Size of message buffer from client
#define PORT 1234
#define IP "127.0.0.1"
#define LISTSIZE 5 // Number of recent messages to save

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
    int num = 0;                  // Number of messages received
    char message[BUFSIZE];        // Array to hold message

    char list[LISTSIZE][BUFSIZE] = {0}; // Array to hold last 5 messages received 
    time_t now;                   // Holds current local time
    struct tm *tinfo;             // struct to hold time information
    char *time_str;               // String to hold time information

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
        
        /** Get/decrypt/display message */
        // TODO
        
        sprintf(message, "Message #%d\n%s: IP:%s\n", num+1, time_str, inet_ntoa(client.sin_addr));
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


        /* Send encrypted list to client */
        system("clear"); // clear terminal screen
        printf("Line %d\n", strcmp(buf, "\n"));
        for (int j = 0; j < LISTSIZE; j++) { 
            printf("%s\n", list[j]); // Print to terminal screen
            // TODO encrypt and send payload to client

            if (sendto(sd, list[j], strlen(list[j]), 0, (struct sockaddr*)&client, client_len) < 0) { 
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
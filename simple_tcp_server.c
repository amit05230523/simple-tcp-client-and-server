
/*
 * License:
 *
 * This file has been released under "unlicense" license
 * (https://unlicense.org).
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or distribute
 * this software, either in source code form or as a compiled binary, for any
 * purpose, commercial or non-commercial, and by any means.
 *
 * For more information about this license, please visit - https://unlicense.org
 */

/*
 * Author: Amit Choudhary
 * Email: amitchoudhary0523 AT gmail DOT com
 */

#include "simple_tcp_client_server_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

static void print_usage_and_exit(int argc, char *argv[], FILE *out);
static char *get_input_from_stdin_and_discard_extra_characters(char *str,
                                        int size, int min_size, int max_size);
static void recv_and_send(int newsockfd, FILE *out);

static void recv_and_send(int newsockfd, FILE *out)
{

    char msg[MAX_MSG_SIZE] = {0};
    int retval_int = -1;
    char *retval_char_ptr = NULL;

    fprintf(out, "\nWaiting for a message from the client...\n");

    while (1) {

        memset(msg, 0, MAX_MSG_SIZE);

        retval_int = (int)(recv(newsockfd, msg, MAX_MSG_SIZE, 0));

        msg[MAX_MSG_SIZE - 1] = '\0';

        fprintf(out, "\nMessage received from the client: %s\n", msg);

        if (retval_int < 0) { // some error happened
            fprintf(out, "\nError: recv() returned error: %s. Closing the"
                         " server socket.\n", strerror(errno));
            close(newsockfd);
            return;
        }

        if (retval_int == 0) { // client socket is closed
            fprintf(out, "\nError: Client socket is closed. Closing the server"
                         " socket.\n");
            close(newsockfd);
            return;
        }

        fprintf(out, "\nPlease input a message to send to the client(max %d"
                     " characters else the input message will be truncated to"
                     " %d characters): ", MAX_MSG_SIZE - 1, MAX_MSG_SIZE - 1);

        memset(msg, 0, MAX_MSG_SIZE);

        retval_char_ptr = get_input_from_stdin_and_discard_extra_characters(msg,
                                    MAX_MSG_SIZE, MIN_MSG_SIZE, MAX_MSG_SIZE);

        if (retval_char_ptr == NULL) {
            fprintf(out, "\nError: No input received. Sending null byte to the"
                         " client.\n");
            msg[0] = '\0';
        }

        // At this point, msg is null terminated, so strlen can be used safely.
        // Extra 1 byte for null terminating character.
        retval_int = (int)(send(newsockfd, msg, strlen(msg) + 1, 0));

        if (retval_int < 0) { // some error happened
            fprintf(out, "\nError: send() returned error: %s. Closing the"
                         " server socket.\n", strerror(errno));
            close(newsockfd);
            return;
        }

        fprintf(out, "\nMessage sent to the client. Waiting for a message from"
                     " the client...\n");

    } // end of outer while (1) loop

    return;

} // end of function recv_and_send()

int main(int argc, char *argv[])
{

    FILE *out = stdout;
    int sockfd = -1;
    int newsockfd = -1;
    struct sockaddr_in serv_addr, client_addr;
    int client_addr_len = -1;
    int reuse_addr = 1;
    char client_ipv4_address[IPv4_ADDR_LEN] = {0};

    if (argc != 3) {
        print_usage_and_exit(argc, argv, out);
    }

    //create socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(out, "\nError: Could not create socket: %s. Exiting..\n\n",
                strerror(errno));
        exit(1);
    }

    // Set SO_REUSEADDR on the socket so that the server's address is available
    // immediately to reuse, in case the server restarts or is restarted.
    // Normally, the TCP connection will enter TIME_WAIT/CLOSE_WAIT state on
    // connection close and the server's address will not be available
    // immediately for reuse. But setting SO_REUSEADDR on the socket makes the
    // server's address available immediately for reuse. For setting
    // SO_REUSEADDR, setsockopt() should be called before bind(). Actually, it
    // is best to call setsockopt() just after the socket has been created using
    // the socket() call.
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                   (const char *)(&reuse_addr), sizeof(reuse_addr)) < 0) {
        fprintf(out, "\nError: Could not set SO_REUSEADDR on the socket: %s."
                     " Exiting..\n\n", strerror(errno));
        exit(1);
    }

    // fill in the server's address
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons((uint16_t)(atoi(argv[2])));

    // bind the server's address to the socket
    if (bind(sockfd, (struct sockaddr *)(&serv_addr), sizeof(serv_addr)) < 0) {
        fprintf(out, "\nError: Could not bind the server's address to the"
                     " socket: %s. Exiting..\n\n", strerror(errno));
        exit(1);
    }

    // listen() call is necessary because without listen() call, this socket
    // won't become a server socket and hence it will not accept any
    // connections.
    if (listen(sockfd, 5) < 0) {
        fprintf(out, "\nError: listen() call failed: %s. Exiting..\n\n",
                strerror(errno));
        exit(1);
    }

    memset(&client_addr, 0, sizeof(client_addr));
    client_addr_len = sizeof(client_addr);

    while (1) {

        fprintf(out, "\nWaiting for a new connection...\n");

        // accept new incoming connection
        newsockfd = accept(sockfd, (struct sockaddr *)(&client_addr),
                           (socklen_t *)(&client_addr_len));

        if (newsockfd < 0)
        {
            fprintf(out, "\nError: accept() call failed: %s. Exiting..\n\n",
                    strerror(errno));
            exit(1);
        }

        fprintf(out, "\nGot a new connection.\n");

        if (inet_ntop(AF_INET, &client_addr.sin_addr.s_addr,
                      client_ipv4_address, IPv4_ADDR_LEN) != NULL) {
            client_ipv4_address[IPv4_ADDR_LEN - 1] = '\0';
            fprintf(out, "\nClient's IPv4 address: %s\n", client_ipv4_address);
            fprintf(out, "Client's port number: %d\n",
                    ntohs(client_addr.sin_port));
        }

        // communicate with the client
        recv_and_send(newsockfd, out);

    } // end of while(1) loop

    // close the socket
    close(sockfd);

    return 0;

} // end of function main()

static void print_usage_and_exit(int argc, char *argv[], FILE *out)
{

    if (argc != 3) {
        fprintf(out, "\nError: Incorrect usage.\n\n");
        fprintf(out, "USAGE: %s server_ipv4_address server_port_number\n\n",
                argv[0]);
        fprintf(out, "This program takes two arguments. The first is the TCP"
                     " server's IPv4 address and the second is the port number"
                     " on which the TCP server will listen for new"
                     " connections.\n\n");
        fprintf(out, "This program doesn't check the validity of the arguments."
                     " So, please give valid arguments else this program may"
                     " behave in unexpected manner and it may crash also.\n\n");
        fprintf(out, "Please try again. Exiting..\n\n");
        exit(1);
    }

} // end of function print_usage_and_exit()

/*
 * get_input_from_stdin_and_discard_extra_characters():
 *
 *      Function get_input_from_stdin_and_discard_extra_characters() reads at
 *      most (size - 1) characters from stdin and stores them in 'str'.
 *      One character is used to null terminate 'str'. The rest of the
 *      remaining characters in stdin are read and discarded, they are not
 *      stored in 'str'. So, when this function returns then there is no
 *      input/characters left in stdin.
 *
 *      If 'str' is NULL then it is an error and nothing is read from stdin and
 *      NULL is returned.
 *
 *      If 'size' is greater than 'max_size' or less than 'min_size' then it is
 *      an error and nothing is read from stdin and NULL is returned.
 */
static char *get_input_from_stdin_and_discard_extra_characters(char *str,
                                        int size, int min_size, int max_size)
{

    int c = 0;
    int i = 0;

    if (str == NULL) {
        return NULL;
    }

    if ((size < min_size) || (size > max_size)) {
        return NULL;
    }

    for (i = 0; i < (size - 1); i = i + 1) {

        c = getchar();

        if ((c == '\n') || (c == EOF)) {
            str[i] = 0;
            return str;
        }

        str[i] = (char)(c);

    } // end of for loop

    str[i] = 0;

    // discard the rest of the input
    while ((c = getchar()) && (c != '\n') && (c != EOF));

    return str;

} // end of function get_input_from_stdin_and_discard_extra_characters()



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
static void send_and_recv(int sockfd, FILE *out);

static void send_and_recv(int sockfd, FILE *out)
{

    char msg[MAX_MSG_SIZE] = {0};
    int retval_int = -1;
    char *retval_char_ptr = NULL;

    while (1) {

        fprintf(out, "\nPlease input a message to send to the server(max %d"
                     " characters else the input message will be truncated to"
                     " %d characters): ", MAX_MSG_SIZE - 1, MAX_MSG_SIZE - 1);

        memset(msg, 0, MAX_MSG_SIZE);

        retval_char_ptr = get_input_from_stdin_and_discard_extra_characters(msg,
                                    MAX_MSG_SIZE, MIN_MSG_SIZE, MAX_MSG_SIZE);

        if (retval_char_ptr == NULL) {
            fprintf(out, "\nError: No input received. Sending null byte to the"
                         " server.\n");
            msg[0] = '\0';
        }

        // At this point, msg is null terminated, so strlen can be used safely.
        // Extra 1 byte for null terminating character.
        retval_int = (int)(send(sockfd, msg, strlen(msg) + 1, 0));

        if (retval_int < 0) { // some error happened
            fprintf(out, "\nError: send() returned error: %s.\n",
                    strerror(errno));
            return;
        }

        fprintf(out, "\nMessage sent to the server. Waiting for a message from"
                     " the server...\n");

        memset(msg, 0, MAX_MSG_SIZE);

        retval_int = (int)(recv(sockfd, msg, MAX_MSG_SIZE, 0));

        msg[MAX_MSG_SIZE - 1] = '\0';

        fprintf(out, "\nMessage received from the server: %s\n", msg);

        if (retval_int < 0) { // some error happened
            fprintf(out, "\nError: recv() returned error: %s.\n",
                    strerror(errno));
            return;
        }

        if (retval_int == 0) { // server socket is closed
            fprintf(out, "\nError: Server socket is closed.\n");
            return;
        }

    } // end of outer while (1) loop

    return;

} // end of function recv_and_send()

int main(int argc, char *argv[])
{

    FILE *out = stdout;
    int sockfd = -1;
    struct sockaddr_in server;

    if (argc != 3) {
        print_usage_and_exit(argc, argv, out);
    }

    fprintf(out, "\n");

    //create socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(out, "\nError: Could not create socket: %s. Exiting..\n\n",
                strerror(errno));
        exit(1);
    }

    // fill in the server's address
    memset(&server, 0, sizeof(server));
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_family = AF_INET;
    server.sin_port = htons((uint16_t)(atoi(argv[2])));

    fprintf(out, "Initiating connection with the server...\n");

    // connect to server
    if (connect(sockfd, (struct sockaddr *)(&server), sizeof(server)) < 0) {
        fprintf(out, "\nError: Could not connect to server: %s. Exiting..\n\n",
                strerror(errno));
        exit(1);
    }

    fprintf(out, "\nConnected with the server...\n");

    // communicate with the server
    send_and_recv(sockfd, out);

    // close the socket
    fprintf(out, "\nClosing the client socket.\n\n");

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
                     " server's IPV4 address and the second is the port number"
                     " on which the TCP server is listening for new"
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


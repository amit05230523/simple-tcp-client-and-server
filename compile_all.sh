#!/bin/bash

set -x

rm -f simple_tcp_client.out

rm -f simple_tcp_server.out

gcc -Wall -Werror -Wextra -Wundef -Wunreachable-code -Winit-self -Wparentheses -Wconversion -Wsign-conversion -Wsign-compare -Werror-implicit-function-declaration -Wmissing-prototypes -Wmissing-declarations -Wformat-security simple_tcp_client.c -o simple_tcp_client.out

gcc -Wall -Werror -Wextra -Wundef -Wunreachable-code -Winit-self -Wparentheses -Wconversion -Wsign-conversion -Wsign-compare -Werror-implicit-function-declaration -Wmissing-prototypes -Wmissing-declarations -Wformat-security simple_tcp_server.c -o simple_tcp_server.out


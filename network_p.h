#ifndef NETWORK_P_H
#define NETWORK_P_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "network.h"

#define SEND 0
#define RECV 1

int put_to_remote( int, const char* const );
int get_from_remote( int, const char* const );

int receipt_confirmation( int, int );

#endif

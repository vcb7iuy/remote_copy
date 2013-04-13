#ifndef NETWORK_H
#define NETWORK_H

#define OPTIONSIZE 4
#define BUFFERSIZE 256

#define ERROR -1
#define SUCCESS 0

typedef struct {
  char option[OPTIONSIZE];
  char filename[BUFFERSIZE];
}COMMAND;

int connect_to_server( COMMAND );
void connect_from_client( void );



#endif

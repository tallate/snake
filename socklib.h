#ifndef _SOCKLIB_H_
#define _SOCKLIB_H_

int make_server_socket(int portnum);

int connect_to_server(char *host, int portnum);

#endif

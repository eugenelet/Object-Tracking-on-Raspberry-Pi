/* UDP client in the internet domain */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

#define DATAGRAM_SIZE	8

using namespace std;

static int sock, n;
static unsigned int length;
static struct sockaddr_in server, from;
static struct hostent *hp;
void error_transmit(const char *);

int transmit_init(char *ip_addr, char* port)
{
	sock = socket(AF_INET, SOCK_STREAM, 0);
	//int doReuse = 1;
	//setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
//					           (const char *)&doReuse, sizeof(doReuse)); 
   if (sock < 0) error_transmit("socket");

   server.sin_family = AF_INET;
   hp = gethostbyname(ip_addr);
   if (hp==0) error_transmit("Unknown host");

   bcopy((char *)hp->h_addr, 
        (char *)&server.sin_addr,
         hp->h_length);
   server.sin_port = htons(atoi(port));
   length=sizeof(struct sockaddr_in);

    while ( connect(sock,(struct sockaddr *) &server,sizeof(server)) < 0 ){
			std::cout << "ERROR connecting:" << endl;
	}
       // error_transmit("ERROR connecting");

	std::cout << "Connection" << port << "Accepted!" << std::endl;
}

int transmit(unsigned char* data){

	n = write(sock,data, DATAGRAM_SIZE);
	if (n < 0) error_transmit("Write");
   
//	close(sock);
}

void close_transmit(){
	close(sock);
}

void error_transmit(const char *msg)
{
    perror(msg);
    exit(0);
}

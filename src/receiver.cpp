/* Creates a datagram server.  The port 
   number is passed as an argument.  This
   server runs forever */

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#define DATAGRAM_SIZE 8

static int sock, length, n, newsockfd;
static socklen_t fromlen;
static struct sockaddr_in server;
static struct sockaddr_in from;
static unsigned char recvPacket[DATAGRAM_SIZE];

void error_receiver(const char *msg)
{
	perror(msg);
	exit(0);
}

void receiver_init(char *port)
{
	//if (argc < 2) {
	// 	fprintf(stderr, "ERROR, no port provided\n");
	// 	exit(0);
	//}
	
//	sock=socket(AF_INET, SOCK_DGRAM, 0);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) error_receiver("Opening socket");
	int doReuse = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
					           (const char *)&doReuse, sizeof(doReuse)); 
	length = sizeof(server);
	bzero(&server,length);
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=INADDR_ANY;
	server.sin_port=htons(atoi(port));
	if (bind(sock,(struct sockaddr *)&server,length)<0) 
		error_receiver("binding");
	fromlen = sizeof(struct sockaddr_in);
     listen(sock,2);
	 //	write(1,"Received a datagram: ",21);
	 //	 snprintf(buf, 1024,"%u",recvPacket[0]);
	 //	write(1,buf,n);
	// 	n = sendto(sock,"Got your message\n",17,
	// 	           0,(struct sockaddr *)&from,fromlen);
	// 	if (n  < 0) error_receiver("sendto");
	std::cout << "Waiting for connections... (PORT: " << port << ")" <<std::endl; 
     newsockfd = accept(sock,
             (struct sockaddr *) &from,
             &fromlen);
	std::cout << "Connection " << port  << " Accepted!" << std::endl;
}

unsigned char* receiver(){
//	n = recvfrom(sock,recvPacket,1024,0,(struct sockaddr *)&from,&fromlen);

	//n = read(newsockfd,recvPacket,1023);
	n = read(newsockfd,recvPacket,DATAGRAM_SIZE);
	if (n < 0) error_receiver("Read");
//	cout << hex << recvPacket[0]<<endl;
	//close(newsockfd);
	return recvPacket;
}

void close_receiver(){
	close(newsockfd);
}

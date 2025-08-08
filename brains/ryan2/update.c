#include "update.h"
#include "brain.h"
#include "actionqueue.h"

/* Update.c is a function that checks for updates to the program. **/

#define ADDRESS "ryan2.servehttp.com"
#define PORT 80
#define COMMAND_START "GET /updates/"
#define COMMAND_END ".html\n\r"


#ifdef WIN32



#include <windows.h>
#include <stdio.h>







int doUpdateCheck(char *buf, int maxdatasize) {
	WSADATA ws;
	int d;
	SOCKET s;
	struct sockaddr_in a;
	struct hostent *h;
	int numbytes;
	
	d = WSAStartup(0x0101, &ws);
	s = socket(AF_INET, SOCK_STREAM, 0);
	a.sin_family = AF_INET;
	a.sin_port = htons(PORT);
	h = gethostbyname(ADDRESS);
	a.sin_addr.s_addr = *((unsigned long *) h->h_addr);
	
	d = connect(s, (struct sockaddr *)&a, sizeof(a));
	
	send(s, COMMAND_START VERSION COMMAND_END, sizeof(COMMAND_START VERSION COMMAND_END), 0);
	
	
	if ((numbytes=recv(s, buf, maxdatasize-1, 0)) == -1) {
		return 0;
	}

	buf[numbytes] = '\0';
	closesocket(s);
	WSACleanup();
	
	return 1;
}






#else
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdarg.h>









int doUpdateCheck(char *buf, int maxdatasize) {
	
	int sockfd, numbytes;  
	struct hostent *he;
	struct sockaddr_in their_addr; // connector's address information 


	printf("checkupdate\n");
	if ((he=gethostbyname(ADDRESS)) == NULL) {
	printf("Can't get host name\n");
		return FALSE;
	}
	

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Can't get socket\n");
	}

	their_addr.sin_family = AF_INET;	// host byte order 
	their_addr.sin_port = htons(PORT);  // short, network byte order 
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct 

	if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
		printf("Can't Connect");
	}

	send(sockfd, COMMAND_START VERSION COMMAND_END, sizeof(COMMAND_START VERSION COMMAND_END), 0);
	if ((numbytes=recv(sockfd, buf, maxdatasize-1, 0)) == -1) {
		
	}

	buf[numbytes] = '\0';


	close(sockfd);

	return TRUE;
}

#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <resolv.h>

/*Created by Yunqiao Yang
*Date Feb 17, 2017
*Description: This is the client side of 656 project, which can connect with the server side and communicate.
*/

#define MAXBUF 1024

int main(int argc, char* argv[])
{
	int sockfd,len;
	struct sockaddr_in dest;
	char buffer[MAXBUF + 1];
	if(argc != 3)
	{
		printf("Error format. It must be : \n\t\t%s IP port\n", argv[0]);
		exit(EXIT_FAILURE);

	}
	//then create the socker object
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Socket");
		exit(errno);

	}
	printf("socket created...\n");
	bzero(&dest, sizeof(dest));         //
	dest.sin_family = AF_INET;       //IPV4
	dest.sin_port = htons(atoi(argv[2])); // the port number of the address
	if(inet_aton(argv[1], (struct in_addr*)&dest.sin_addr.s_addr) == 0) // the server's IP address
	{
		perror(argv[1]);
		exit(errno);

	}
	if(connect(sockfd,(struct sockaddr*)&dest, sizeof(dest)) == -1)
	{
		perror("Connection");
		exit(errno);

	}
	printf("Connect server successfully!\n");
	pid_t pid;
	if((pid = fork()) == -1)   // create the child process
	{
		perror("fork");
		exit(EXIT_FAILURE);	

	}
	else if(pid == 0)              //the child process is been used for receiving message from server
	{
		while(1)
		{
			bzero(buffer, MAXBUF + 1);
			len = recv(sockfd, buffer, MAXBUF, 0);
			if(len > 0)
			{
				printf("received successfully: '%s', %d byte receieved.\n", buffer, len);

			}
			else if(len < 0)
			{
				perror("receive");
				break;

			}
			else
			{
				printf("The other one closed, quit.\n");
				break;

			}

		}

	}
	else                     // the parent process is used for sending message to server
	{
		while(1)
		{
			bzero(buffer, MAXBUF + 1);
			printf("Please input a message to server.\n");
			fgets(buffer, MAXBUF, stdin);
			if(!strncasecmp(buffer, "quit", 4))
			{
				printf("I will quit.\n");
				break;
				
			}
			len = send(sockfd, buffer, strlen(buffer) - 1, 0);
			if(len < 0)
			{
				perror("send");
				break;

			}


		}

	}
	close(sockfd);
	


	return 0;
}

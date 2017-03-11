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
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <termio.h>


/*
*Created by Yunqiao Yang
*Date Feb 17, 2017
*Modified by Yunqiao Yang
*Date: March 3, 2017
*Description: This is the server side of the project. Bying implementing it using TCP, it can receive the messages from the client and then send the command to Arduino 
*board.
*Description2: This is the refined server side. The main functionality is receiving the message from the clients
*, which is recognized previously. And then the command will be sent to the serial port ACM0, Arduino.
*/


#define MAXBUF 1024

int portCommunication(void);  //the functionality of communicating with serial port.
void addLog(char *logtext, char *ip);  //the functionality of keeping log file

int portCommunication(void)
{
	int fd;
	
	fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_NONBLOCK);
	if(fd == -1)
	{
		printf("cannot open the serial port...\n");
		return -1;
	}
	
	else
	{
		//printf("open serial port succeed...\n");
		fcntl(fd, F_SETFL, 0);
		return fd;
	}
}

void addLog(char *logtext, char *ip)
{
	FILE *fp;
	if(!(fp = fopen("log.txt", "a+")))
	{
		printf("cannot open the log file. \n");
		exit(0);
	}
	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = localtime (&rawtime);
	printf("%s", asctime(timeinfo));
	fseek(fp, 0, SEEK_END);
	fwrite(ip,strlen(ip), 1, fp);
	fprintf(fp, "\t");
	fwrite(logtext, strlen(logtext), 1, fp);
	fprintf(fp, "\t");
	fwrite(asctime(timeinfo), strlen(asctime(timeinfo)), 1, fp);
	fprintf(fp, "\n");
	fclose(fp);
	
}


int main(int argc , char* argv[])
{
	int pid; // preocess ID
	int sockfd, new_fd;
	socklen_t len;
	struct sockaddr_in my_addr, their_addr;
	unsigned int myport, lisnum;     //the port and the number of listening queue
	char buf[MAXBUF + 1];
	int fd; //this parameter is used to communicate with serial port.

	if (argv[2])
		myport = atoi(argv[2]);  // changing the command line to integer, which can be used into the port.
	else
		myport = 10101; // the default port is 10101

	
	if(argv[3])
		lisnum = atoi(argv[3]); //this is the number of listening queue.
	else
		lisnum = 5; // the default number of listening queue 


	// then create the socket object
	if ((sockfd = socket(AF_INET, SOCK_STREAM , 0)) == -1)
	{
		// failed to create the socket object
		perror("socket");
		exit(EXIT_FAILURE);

	}

	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = AF_INET;  //IPV4
	my_addr.sin_port = htons(myport); //the port number
	
	if(argv[1])
	{
		my_addr.sin_addr.s_addr = inet_addr(argv[1]);
	}

	else
		my_addr.sin_addr.s_addr = INADDR_ANY; // any address of this machine

	// then bind the information of the address
	if (bind(sockfd, (struct sockaddr*) &my_addr, sizeof(struct sockaddr)) == -1)
	{
		perror("bind");
		close(sockfd);
		//close(new_fd);
		exit(EXIT_FAILURE);
	}	

	// then listen from the network
	if(listen(sockfd, lisnum) == -1)
	{
		perror("listen");
		close(sockfd);
		exit(EXIT_FAILURE);

	}
	printf("wait for connecting...\n");
	if((new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &len)) == -1)
	{
		//block waiting for connection, when it becomes succeed, returen new socket descriptor, their_Addr is used to store the sockaddr of client side
		perror("accept");
		close(sockfd);
		close(new_fd);
		exit(EXIT_FAILURE);

	}
	else
		printf("server: Got connection from %s, port %d, socket %d\n", inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port), new_fd);


	// the function fork() is used to create a new process to handle the current connection when there is a connection requirement
	if((pid = fork()) == -1)   // failed to create a new process
	{
		perror("fork");
		close(sockfd);
		close(new_fd);
		exit(EXIT_FAILURE);

	}
	else if(pid == 0)    // the child process is used to receieve message from the client
	{
		while(1)
		{
			bzero(buf, MAXBUF + 1);
			len = recv(new_fd, buf, MAXBUF, 0);
			if(len > 0)
			{
				printf("Message received successful: '%s', %d Byte received.\n", buf, len);
				char *delim = "/";
				char *logtext = NULL;
				char *command = NULL;
	
				
	
				logtext = strtok(buf, delim);
				
				//printf("%s\n", logtext);
				command = strtok(NULL, delim);
				//printf("%s\n", command);
				
				addLog(logtext, inet_ntoa(their_addr.sin_addr));
				
				printf("Now sending message to Arduino board...\n");
				fd = portCommunication();
				write(fd, command, strlen(command));
				//bzero(buf, MAXBUF + 1);
				//read(fd, buf, MAXBUF + 1);
				//printf("\n%s\n", buf);
				close(fd);
				printf("Done...\n");
			
			}
			else if(len < 0)
			{
				printf("Receieving message failed. Error code is: %d, error message is '%s'\n", errno, strerror(errno));
				break;

			}
			else
			{
				printf("The other one closed and quited.\n");
				break;

			}

		}
		
	}
	else       // the parent process is used to listening other clients
	{
		
		
		while(1)
		{
			if((pid = fork()) == -1)
			{
				perror("fork");
				close(sockfd);
				close(new_fd);
				exit(EXIT_FAILURE);
			}
			
			else if (pid > 0)   //the infinite fork() to handle more clients...
			{
				if((new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &len)) == -1)
				{
				//block waiting for connection, when it becomes succeed, returen new socket descriptor, their_Addr is used to store the sockaddr of client side
					perror("accept");
					close(sockfd);
					close(new_fd);
					exit(EXIT_FAILURE);

				}
				else
					printf("server: Got connection from %s, port %d, socket %d\n", inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port), new_fd);
			}
			
			else   //the child process is used to receeving the certain commands
			{
				while(1)
				{
					bzero(buf, MAXBUF + 1);
					len = recv(new_fd, buf, MAXBUF, 0);
					if(len > 0)
					{
						printf("Commands from %s, port %d, socket %d\n", inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port), new_fd);
						printf("Message received successful: '%s', %d Byte received.\n", buf, len);
						char *delim = "/";
						char *logtext = NULL;
						char *command = NULL;
				
	
						logtext = strtok(buf, delim);
						//printf("%s\n", logtext);
						
						addLog(logtext, inet_ntoa(their_addr.sin_addr));
						
						command = strtok(NULL, delim);
						//printf("%s\n", command);
						printf("Now sending message to Arduino...\n");
						fd = portCommunication();
						write(fd, command, strlen(command));
						//bzero(buf, MAXBUF + 1);
						//read(fd, buf, MAXBUF + 1);
						//printf("\n%s\n", buf);
						close(fd);
						printf("Done...\n");
						
					}
					else if(len < 0)
					{
						printf("Receieving message failed. Error code is: %d, error message is '%s'\n", errno, strerror(errno));
						break;

					}
					else
					{
						//printf("The other one closed and quited.\n");
						break;

					}
			
				}
			}
		}
		

	}
	
	close(new_fd);
	close(sockfd);

	

	return 0;

}

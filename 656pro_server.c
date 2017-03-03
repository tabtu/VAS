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


/*
*Created by Yunqiao Yang
*Date Feb 17, 2017
*Description: This is the server side of teh project voice control application which can recognize the specifiec commands so that it can control the Arduino hardware
*/


#define MAXBUF 1024

void commandRecog(char string[]); // declaration of command line recognition




void commandRecog(char string[])   //command line recognition function

{
	
	//char string[200],
	char matchcase[4][10];
	FILE *fp;  //file pointer to write time stamp and string log to log.txt  

    	//define each key ..
    	strcpy(matchcase[0], "open");
    	strcpy(matchcase[1], "light");
    	strcpy(matchcase[2], "close");
    	strcpy(matchcase[3], "dark");
    	int i = 0, j = 0;

 

      	/*Scanning string*/
	
    //	printf("Enter string: ");

   	 //do

    	//{

       		//fflush(stdin);

        	//c = getchar();

        	//string[i++] = tolower(c);

 

    	//} 
    	//while (c != '\n');

    	//string[i - 1] = '\0';
	
	printf("\nThe target string is: %s\n\n", string);

	fp = fopen("log.txt", "a+");

	if(fp == 0)
	{
		printf("Cannot open the log file.\n");
		exit(0);

	}

	

	fseek(fp, 0, SEEK_END);

	fwrite(string, strlen(string), 1, fp);

	fprintf(fp,"\t");	

	time_t timep;

	time(&timep);

	printf("Inputing time:%s\n" , ctime(&timep));

	fwrite(ctime(&timep), strlen(ctime(&timep)), 1, fp);

	fprintf(fp,"\n");

	fclose(fp);

    	/*Scanning substring*/

    	/*printf("Enter key: ");

    	i = 0;

    	do

    	{

        	fflush(stdin);

        	c = getchar();

        	matchcase[i++] = tolower(c);

    	} 
    	while (c != '\n');

    	matchcase[i - 1] = '\0';*/

    	int counter;   // this is the parameter to count the times to recognition
    	for(counter = 0; counter < 4; counter++)
  	 	{
    		for (i = 0,j = 0; i < strlen(string) - strlen(matchcase[counter]) + 1; i++)

    		{

       			int index = i;

        		if (string[i] == matchcase[counter][j])

        		{

           			 do

            			{

               				i++;
                			j++;

            			} while(j != strlen(matchcase[counter]) && string[i] == matchcase[counter][j]);

           			if (j == strlen(matchcase[counter]))

            			{

					printf("%s:\n", matchcase[counter]);               	 			
					printf("Match found from position %d to %d.\n", index + 1, i);

                			break;

            			}

           		 	else

            			{

                			i = index + 1;

                			j = 0;

            			}

       	 		}
  		}

    	}

    	//printf("No substring match found in the string.\n");

 

    	//return 0;

}


int main(int argc , char* argv[])
{
	int pid; // preocess ID
	int sockfd, new_fd;
	socklen_t len;
	struct sockaddr_in my_addr, their_addr;
	unsigned int myport, lisnum;     //the port and the number of listening queue
	char buf[MAXBUF + 1];

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
		exit(EXIT_FAILURE);
	}	

	// then listen from the network
	if(listen(sockfd, lisnum) == -1)
	{
		perror("listen");
		exit(EXIT_FAILURE);

	}
	printf("wait for connecting...\n");
	if((new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &len)) == -1)
	{
		//block waiting for connection, when it becomes succeed, returen new socket descriptor, their_Addr is used to store the sockaddr of client side
		perror("accept");
		exit(EXIT_FAILURE);

	}
	else
		printf("server: Got connection from %s, port %d, socket %d\n", inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port), new_fd);


	// the function fork() is used to create a new process to handle the current connection when there is a connection requirement
	if((pid = fork()) == -1)   // failed to create a new process
	{
		perror("fork");
		exit(EXIT_FAILURE);

	}
	else if(pid == 0)    // the child process is used to receieve message from the client
	{
		/*while(1)
		{
			bzero(buf, MAXBUF + 1);
			printf("Please input the message to send: \n");
			fgets(buf, MAXBUF + 1, stdin);
			if(!strncasecmp(buf, "quit", 4))
			{
				printf("I will close the connection!\n");
				break;

			}
			len = send(new_fd, buf, strlen(buf) - 1, 0); // send the message to the client
			if(len < 0)
			{
				printf("message '%s' send failed! error code is %d, error message is %s.\n", buf, errno, strerror(errno));
				break;

			}
	

		}*/
		
		while(1)
		{
			bzero(buf, MAXBUF + 1);
			len = recv(new_fd, buf, MAXBUF, 0);
			if(len > 0)
			{
				printf("Message received successful: '%s', %d Byte received.\n", buf, len);
			
			}
			else if(len < 0)
			{
				printf("REceieving message failed. Error code is: %d, error message is '%s'\n", errno, strerror(errno));
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
		/*while(1)
		{
			bzero(buf, MAXBUF + 1);
			len = recv(new_fd, buf, MAXBUF, 0);
			if(len > 0)
				printf("Message received successful: '%s', %d Byte received.\n", buf, len);
			else if(len < 0)
			{
				printf("REceieving message failed. Error code is: %d, error message is '%s'\n", errno, strerror(errno));
				break;

			}
			else
			{
				printf("The other one closed and quited.\n");
				break;

			}
			
		}*/
		
		while(1)
		{
			if((pid = fork()) == -1)
			{
				perror("fork");
				exit(EXIT_FAILURE);
			}
			
			else if (pid > 0)
			{
				if((new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &len)) == -1)
				{
				//block waiting for connection, when it becomes succeed, returen new socket descriptor, their_Addr is used to store the sockaddr of client side
					perror("accept");
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
						commandRecog(buf);
					}
					else if(len < 0)
					{
						printf("REceieving message failed. Error code is: %d, error message is '%s'\n", errno, strerror(errno));
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

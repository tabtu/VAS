VAS Server Coding.
There are totally two C source files. One is the 656pro_server.c and the another one is the 656pro_client.c

Fot the server:
  After compiling, there should be two parameters. one is the ip address and the another one is the port number,which means that when open the server, the administrator should acknowledge the ip address of this machine and the specific port that he want to use. When the network is unstable, when running this server, there could be a error of accept: invalid argument. So when this happened, the administrator should rerun the program again. Sometimes there could be error of bind: the ip address is already used. This is because the previous unsuccessful operation took up the ip address and if you wait one minute it will be ok. But at most time, these two errors will not present.
   The functionality of the server is to accept the command from all clients and send it respectively to board. In next iteration, there will be a functionality of log file which can take the log history of each client's commands.
For the testing client: (command line client)
  There should be two parameters as well, just like the server.
  The key word recognition will be implemented in the next iteration because when implementing this in server side in C, the efficiency could be very low so that it cannot control the devices in time.

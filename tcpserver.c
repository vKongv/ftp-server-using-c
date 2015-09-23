/*tcpserver.c*/
/* Version 0.1 
   Allows user to read file from server by providing the file name / path
*/
#include "inet.h"
#define BUFSIZE 1024

int main () {
  /**
  sockfd = Listening socket
  new_sockfd = Processing socket
  clilen = Client address's length
  buffer = To store data
  serv_addr = Structure of sockaddr_in of Server IP address
  cli_addr = Structure of sockaddr_in of Client IP address
  **/
  int sockfd, new_sockfd, clilen;
  int yes = 1;
  char buffer[BUFSIZE + 1];
  struct sockaddr_in serv_addr, cli_addr;

  // socket() will return an int value. The value is the value for sockfd
  // It is a TCP port because it using type SOCK_STREAM
  printf("Error message is: %s", ERR_MSG);
  if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    perror("Server: socket() error\n");
    exit(1);
  }

  if ( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *) &yes, sizeof(yes) ) == -1){
     perror ("setsockopt error");
   }
  printf("\nEcho Application Demo\n");

  bzero((char *) &serv_addr, sizeof(serv_addr)); /* Clear all the data in serv_addr */
  serv_addr.sin_family = AF_INET; /* It is an IPv4 address*/
  serv_addr.sin_addr.s_addr = inet_addr("192.168.1.253"); /* Local IP address */
  serv_addr.sin_port = htons(SERV_TCP_PORT); //Assign a local port to the address, in this case "25000".

  //Add local protocol address to a socket
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
    perror("Server: bind() error \n");
    exit(1);
  }
  printf("\nWaiting for connection... [bind]\n");
  listen(sockfd, 5); /* Listen to a connection, Max 5 queues */

  clilen = sizeof(cli_addr); /* Get the size of client address */
  new_sockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen); /* Accept the connection and assign a new socket to handle it*/
  if(new_sockfd > 0){
    printf("\nClient connected now.");

  }

  for(;;){
    int fd;
    recv(new_sockfd,buffer,BUFSIZE,0); /* Receive message from client */
    printf("\nReceived message [%s] from CLIENT\n", buffer);

     //If the incoming request is "/q" means user want to quit
    if((strcmp(buffer, "/q") == 0)){
      break;
    }

    //Open the file that specific by client
    if ((fd = open (buffer, 0)) < 0 ){
      send(new_sockfd, "Cannot open the file\n", 25, 0);
      continue;
    }

    bzero (buffer, sizeof(buffer)); /* Clear all the data in the buffer */
    read (fd, buffer, BUFSIZE); //Read the file content into the buffer
    send(new_sockfd, buffer, BUFSIZE,0);  /* Send the file content to client */
  }

  close(new_sockfd);  /* Close the processing socket */
  close(sockfd);  /* Close the listening socket */
  return 0;
}

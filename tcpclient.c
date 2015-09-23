/* tcpclient.c */
/* Version 0.2
   Add menu for client to select
   Tokenize server message and display different result based on server status
*/
#include "inet.h"
#define BUFSIZE 1024

int main (int argc, char *argv[]){
  int sockfd;
  char buffer[BUFSIZE+1];
  struct sockaddr_in serv_addr;

  if (argc <= 1){
    printf("How to use: %s remoteIPaddress [example: ./client 127.0.0.1]\n", argv[0]);
    exit(1);
  }

  bzero ((char *) &serv_addr, sizeof(serv_addr)); /* Clear all the data in serv_addr */
  serv_addr.sin_family = AF_INET; /* It is an IPv4 address*/
  serv_addr.sin_port = htons(SERV_TCP_PORT); /* Assign the server listening port */
  serv_addr.sin_addr.s_addr = inet_addr(argv[1]); // Translate the address into struct format

  // socket() will return an int value. The value is the value for sockfd
  // It is a TCP port because it using type SOCK_STREAM
  if( (sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0){
    perror("Client: socket() error\n");
    exit(1);
  }

  /* Connect to the server */
  if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
    perror("Client: connect() error\n");
    exit(1);
  }

  char *msg[2] = {"SELECTING",""};
  for(;;){

    if ((strcmp(msg[0],"SELECTING")) == 0)
      printf("1. Read file \n2. Write file (Not yet done)\nYour selection: ");
    else if ((strcmp(msg[0],"DISPLAYING")) == 0)
      printf("\nPlease enter the COMPLETE path of the document you want to the SERVER [type /q to quit]\n");
    gets(buffer); /* Get input from user*/
    send (sockfd, buffer, BUFSIZE, 0); /* Send the data to server */
    /* If the incoming request is "/q" means user want to quit */
    //if((strcmp(buffer, "/q") == 0)){
      //break;
    //}
    bzero (buffer, sizeof(buffer)); /* Clear all the data in the buffer */
    recv(sockfd, buffer, BUFSIZE, 0); /* Receive message from server */
    printf("Before strtok: %s\n", buffer);
    char *tempString = strtok(buffer,"^");
    printf("After strtok: %s\n", tempString);
    int i = 0;
      while(tempString){
        printf("Transfering data to msg: %s\n", tempString);
        msg[i] = tempString;
        tempString = strtok(NULL, "");
        i++;
      }
    printf ("Message received from server : \n%s\n", msg[1]); // Print out the message received from server
  }

  close (sockfd); /* Close the connecting port */

  return 0;
}

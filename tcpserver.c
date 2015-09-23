/*tcpserver.c*/
/* Version 0.2
   Able to handle client selection (Currently only implement read function)
   NEW variables, STATUS and currentStatus -> To identify status of server.
   Modify message pattern so it include status of server when sending back to client
*/
#include "inet.h"
#define BUFSIZE 1024
/**
sockfd = Listening socket
new_sockfd = Processing socket
clilen = Client address's length
buffer = To store data
serv_addr = Structure of sockaddr_in of Server IP address
cli_addr = Structure of sockaddr_in of Client IP address
st = To check status of the directory
**/
int sockfd, new_sockfd, clilen;
int yes = 1;
char buffer[BUFSIZE + 1];
struct sockaddr_in serv_addr, cli_addr;
struct stat st = {0};
char *STATUS[] = {"LISTENING", "SELECTING", "DISPLAYING"};
char *currentStatus = "";

int main () {


  // socket() will return an int value. The value is the value for sockfd
  // It is a TCP port because it using type SOCK_STREAM
  printf("Error message is: %s", ERR_MSG);
  if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    perror("Server: socket() error\n");
    exit(1);
  }

  //Create a new directory that used to store user data
  if (stat("user_data", &st) == -1) {
      mkdir("user_data", 0700);
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

  currentStatus = STATUS[0];

  clilen = sizeof(cli_addr); /* Get the size of client address */
  new_sockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen); /* Accept the connection and assign a new socket to handle it*/
  if(new_sockfd > 0){
    printf("\nClient connected now.");

  }

  currentStatus = STATUS[1];
  for(;;){
    int fd, selection = -1;
    if(currentStatus == STATUS[1]){
      printf("\n****************************In Main Menu****************************\n");
      recv(new_sockfd,buffer,BUFSIZE,0); /* Receive message from client */
      printf("\nReceived message [%s] from CLIENT\n", buffer);
      /**
        1 - Read file
        2 - Write file
      **/
      selection = atoi(buffer);
      printf("Integer value of selection: %d\n", selection);
      switch(selection){
        case 1:
          currentStatus = STATUS[2];
          break;
        case 2:
          sprintf(buffer, "%s^%s", currentStatus, "WRTITE FILE is not implement yet...\n");
          send(new_sockfd, buffer, BUFSIZE,0);  /* Send the file content to client */
          break;
        default:
          sprintf(buffer, "%s^%s", currentStatus, "Invalid Selection!\n");
          send(new_sockfd, buffer, BUFSIZE,0);  /* Send the file content to client */
          continue;
      }
      sprintf(buffer, "%s^%s", currentStatus, "Please TYPE a file name to read the file.\n");
      send(new_sockfd, buffer, BUFSIZE,0);  /* Send the file content to client */
    }
    else if (currentStatus == STATUS[2]){
      printf("\n****************************Displaying file content****************************\n");
      recv(new_sockfd,buffer,BUFSIZE,0); /* Receive message from client */
      //If the incoming request is "/q" means user want to go back to selection
      if((strcmp(buffer, "/q") == 0)){
        currentStatus = STATUS[1];
        sprintf(buffer, "%s^%s", currentStatus, "Going back to main selection...\n");
        send(new_sockfd, buffer, BUFSIZE,0);  /* Send the file content to client */
        continue;
      }
      printf("\nReceived message [%s] from CLIENT\n", buffer);
      //Open the file that specific by client
      if ((fd = open (buffer, 0)) < 0 ){
        send(new_sockfd, "Cannot open the file\n", 25, 0);
        continue;
      }
      bzero (buffer, sizeof(buffer)); /* Clear all the data in the buffer */
      read (fd, buffer, BUFSIZE); //Read the file content into the buffer
      printf("Content of file: %s\n", buffer);
      char tempCpy[BUFSIZE];
      printf("Successful create tempCpy\n");
      strcpy(tempCpy,currentStatus);
      printf("Copied from current status\n");
      strcat(tempCpy, "^");
      printf("Combined with ^\n");
      strcat(tempCpy, buffer);
      printf("Combine with buffer\n");
      bzero (buffer, sizeof(buffer)); /* Clear all the data in the buffer */
      strcpy(buffer, tempCpy);
      printf("Message to be send: %s\n", buffer);
      send(new_sockfd, buffer, BUFSIZE,0);  /* Send the file content to client */
    }
    else{
      printf("Unknown server error!");
      send(new_sockfd, "Unknown Error\n", 25, 0);
      break;
    }
    printf("After If\n");
  }

  close(new_sockfd);  /* Close the processing socket */
  close(sockfd);  /* Close the listening socket */
  return 0;
}

/*tcpserver.c*/
/* Version 0.3
    Major update - Allows user to download file from his directory
    Please refer to code for the update
*/
#include "inet.h"
#define BUFSIZE 1024

/**
  Tokenizer list:
    ^ - To combine STATUS of server
    # - To combine FILENAME
    $ - To combine DIRECTORY list
**/
/**
sockfd = Listening socket
new_sockfd = Processing socket
clilen = Client address's length
buffer = To store data
serv_addr = Structure of sockaddr_in of Server IP address
cli_addr = Structure of sockaddr_in of Client IP address
st = To check status of the directory
STATUS[] = Array that contain the status of server
currentStatus = Current status of server
**/
int sockfd, new_sockfd, clilen;
int yes = 1;
char buffer[BUFSIZE + 1];
//char displayMessage[BUFSIZE + 1];
char directory[BUFSIZE + 1];
struct sockaddr_in serv_addr, cli_addr;
struct stat st = {0};
char *STATUS[] = {"LISTENING", "SELECTING", "DOWNLOADING", "UPLOADING"};
char *currentStatus = "";

int loadUserDirectory();
void sendInitialMessage();
int main () {


  // socket() will return an int value. The value is the value for sockfd
  // It is a TCP port because it using type SOCK_STREAM
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
  strcpy(directory, "user_data/directory.txt");

  printf("\nEntering load user dircetories\n");
  if (loadUserDirectory() < 0){
    perror("Cannot load user directory file \n");
    send(new_sockfd, "Bye!\n", 10,0);  /* Send the file content to client */
  }


  for(;;){
    int fd,fddir, selection = -1;
    if(currentStatus == STATUS[1]){
      strcpy(buffer, "1. Download file \n2. Upload file (Not yet done)\n3. Quit\nYour selection: ");
      sendInitialMessage(); // Send initial message
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
          strcpy(buffer, NULL_MSG);
          send(new_sockfd, buffer, BUFSIZE,0);  /* Send a NULL message to client */
          continue;
        case 2:
          currentStatus = STATUS[1];
          strcpy(buffer, "Not yet implement...\n");
          send(new_sockfd, buffer, BUFSIZE,0);  /* Send a NULL message to client */
          continue;
        case 3:
          strcpy(buffer, "Bye!\n");
          send(new_sockfd, buffer, BUFSIZE,0); /* Send a NULL message to client */
          close(new_sockfd);  /* Close the processing socket */
          close(sockfd);  /* Close the listening socket */
          return 0;
        default:
          currentStatus = STATUS[1];
          strcpy(buffer, "Invalid selection!\n");
          send(new_sockfd, buffer, BUFSIZE,0);  /* Send error message to client */
          continue;
      }
      //sprintf(buffer, "%s^%s", currentStatus, "Please TYPE a file name to read the file.\n");
      //send(new_sockfd, buffer, BUFSIZE,0);  /* Send the file content to client */
    }
    else if (currentStatus == STATUS[2]){
      if((fddir = open(directory,0)) < 0){
        perror("Cannot open directory.txt.\n");
        exit(1);
      }

      bzero (buffer, sizeof(buffer)); /* Clear all the data in the buffer */
      printf("Open done..\n");
      read(fddir,buffer,BUFSIZE);
      printf("Read done.. buffer is %s\n", buffer);
      char *tempBuf = (char *) malloc(BUFSIZE);
      strcpy(tempBuf,buffer);
      printf("Copy done.. tempBuf is %s\n",tempBuf);
      strtok(tempBuf,"$");
      printf("Strtok done..tempBuf is %s\n", tempBuf);
      strcpy(buffer,"");
      while(tempBuf){
        strcat(buffer, tempBuf);
        strcat(buffer, "   ");
        tempBuf = strtok(NULL, "$");
      }
      close(fddir);
      printf("Passing to buffer done..\n");
      //Compiling initial message
      char tempMsg[BUFSIZE + 1];
      strcpy(tempMsg, buffer);
      strcat(tempMsg,"\n");
      strcpy(buffer, "Please enter the file NAME you want to the SERVER [type /q to go to previous page]\n\nChoose from the listed file(s):\n\n");
      strcat(buffer, tempMsg);
      printf("Message to be sent: %s\n", buffer);
      //displayMessage = "Please enter the COMPLETE path of the document you want to the SERVER [type /q to go to previous page]\n";
      sendInitialMessage();
      printf("\n****************************Displaying file content****************************\n");
      recv(new_sockfd,buffer,BUFSIZE,0); /* Receive message from client */
      //If the incoming request is "/q" means user want to go back to selection
      if((strcmp(buffer, "/q") == 0)){
        currentStatus = STATUS[1];
        strcpy(buffer, "Going back to main menu...");
        send(new_sockfd, buffer, BUFSIZE,0);  /* Send the file content to client */
        continue;
      }
      printf("\nReceived message [%s] from CLIENT\n", buffer);
      //Open the file that specific by client
      char tempDir[BUFSIZE+1] = "user_data/";
      strcat(tempDir,buffer);
      if ((fd = open (tempDir, 0)) < 0 ){
        printf("Fail fd: %d", fd);
        send(new_sockfd, ERR_MSG[0], 25, 0);
        continue;
      }
      char tempFileName[BUFSIZE + 1];
      strcpy(tempFileName, buffer);
      printf("File name is: %s\n", tempFileName);
      bzero (buffer, sizeof(buffer)); /* Clear all the data in the buffer */
      read (fd, buffer, BUFSIZE); //Read the file content into the buffer
      close(fd);
      printf("File content: %s\n", buffer);
      strcpy(tempMsg, tempFileName);
      strcat(tempMsg, "#");
      strcat(tempMsg, buffer);
      strcpy(buffer, tempMsg);
      //printf("Content of file: %s\n", buffer);
      //char tempCpy[BUFSIZE] = "File content: \n";
      //printf("Successful create tempCpy\n");
      //printf("Copied from current status\n");
      //strcat(tempCpy, "^");
      //printf("Combined with ^\n");
      //strcat(tempCpy, buffer);
      //printf("Combine with buffer\n");
      //bzero (buffer, sizeof(buffer)); /* Clear all the data in the buffer */
      //strcpy(buffer, tempCpy);
      //printf("Message to be send: %s\n", buffer);
      send(new_sockfd, buffer, BUFSIZE,0);  /* Send the file content to client */
    }
    else{
      printf("Unknown server error!");
      send(new_sockfd, ERR_MSG[1], 25, 0);
      break;
    }
    printf("After If\n");
  }

  close(new_sockfd);  /* Close the processing socket */
  close(sockfd);  /* Close the listening socket */
  return 0;
}

int loadUserDirectory(){
  FILE *f;
  f = fopen(directory, "w");

  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir ("user_data")) != NULL) {
    /* print all the files and directories within directory */
    printf("Writing to file \n");
    while ((ent = readdir (dir)) != NULL) {
      if (!(((strcmp(ent->d_name, "."))) == 0 || ((strcmp(ent->d_name, "..")) == 0) || ((strcmp(ent->d_name, "directory.txt")) == 0)))
        fprintf (f, "%s$", ent-> d_name);
    }
    closedir (dir);
  } else {
    /* could not open directory */
    perror ("");
    return -1;
  }
  fclose(f);
  return 0;
}

void sendInitialMessage(){
  strcat(buffer, "^");
  strcat(buffer, currentStatus);
  send(new_sockfd, buffer, BUFSIZE,0);  /* Send the initial message to client */
}

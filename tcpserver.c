/* tcpserver.c */
/* Version 0.4
    Major update - User able to upload file to server's user_data directory
    Other changes:
    - User can download and upload any kind of file. (Before this, limit to 1024 bytes only)
    - Some changes to the old code
    - Improve code readability
*/
#include "inet.h"
#define BUFSIZE 1024

/**
  Tokenizer list:
    ^ - To combine STATUS of server
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
int getFileSize(FILE *);
int fileExists(const char *);

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
      strcpy(buffer, "1. Download file \n2. Upload file \n3. Quit\nYour selection: ");
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
          strcpy(buffer, "OK");
          send(new_sockfd, buffer, BUFSIZE,0);  /* Send a NULL message to client */
          continue;
        case 2:
          currentStatus = STATUS[3];
          strcpy(buffer, "OK");
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
      int tempFSize; //File size
      char *tempFSizeS = (char *) malloc(40); //Char file size
      char *tempFName = (char *) malloc(BUFSIZE + 1); //File name
      char tempPathName[BUFSIZE + 1]; //Path name
      char *tempFCont; //File content

      if((fddir = open(directory,0)) < 0){
        perror("Cannot open directory.txt.\n");
        exit(1);
      }

      bzero (buffer, sizeof(buffer)); /* Clear all the data in the buffer */
      read(fddir,buffer,BUFSIZE);
      char *tempBuf = (char *) malloc(BUFSIZE);
      strcpy(tempBuf,buffer);
      strtok(tempBuf,"$");
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
      sendInitialMessage();
      printf("\n****************************Displaying Downlaod page****************************\n");
      recv(new_sockfd,buffer,BUFSIZE,0); /* Receive message from client */
      //If the incoming request is "/q" means user want to go back to selection
      if((strcmp(buffer, "/q") == 0)){
        currentStatus = STATUS[1];
        strcpy(buffer, "Going back to main menu...");
        sendInitialMessage();
        continue;
      }
      printf("\nReceived message [%s] from CLIENT\n", buffer);
      //Open the file that specific by client
      char tempDir[BUFSIZE+1] = "user_data/";
      strcat(tempDir,buffer);

      FILE *tempFile;
      if ((tempFile = fopen (tempDir, "r+")) == NULL ){
        send(new_sockfd, ERR_MSG[0], 25, 0);
        continue;
      }

      strcpy(tempFName, buffer);
      printf("File name is: %s\n", tempFName);
      send(new_sockfd, tempFName, BUFSIZE,0);  /* Send the file name to client */

      tempFSize = getFileSize(tempFile);
      fclose(tempFile);
      fd = open(tempDir, 0);
      printf("File size is: %d\n", tempFSize);
      sprintf(tempFSizeS, "%d", tempFSize); //Convert file size to string
      send(new_sockfd, tempFSizeS, 40,0);  /* Send the file size to client */

      tempFCont = (char *) malloc(tempFSize); //Allocate file size to the content
      read (fd, tempFCont, tempFSize); //Read the file content into the buffer
      printf("File content: %s\n", tempFCont);
      send(new_sockfd, tempFCont, tempFSize,0);  /* Send the file name to client */
      close(fd);
    }

    else if (currentStatus == STATUS[3]){
      printf("In uploading...\n");
      int tempFSize; //File size
      char *tempFSizeS = (char *) malloc(40); //Char file size
      char *tempFName = (char *) malloc(BUFSIZE + 1); //File name
      char tempPathName[BUFSIZE + 1]; //Path name
      char *tempFCont; //File content

      strcpy(buffer, "Please enter the COMPLETE path of your file that you want to upload: \n");
      printf("Message to be sent: %s\n", buffer);
      sendInitialMessage();
      printf("\n****************************Displaying Upload page****************************\n");
      bzero(buffer,sizeof(buffer));
      recv(new_sockfd, buffer, BUFSIZE, 0); /* Receive file name from server */

      //If the incoming request is "/q" means user want to go back to selection
      if((strcmp(buffer, "/q") == 0)){
        currentStatus = STATUS[1];
        strcpy(buffer, "Going back to main menu...");
        sendInitialMessage();
        continue;
      }
      printf("Receveid file name: %s\n", buffer);
      strcpy(tempFName,buffer);
      recv(new_sockfd, tempFSizeS, 40, 0); /* Receive file size from client */
      tempFSize = atoi (tempFSizeS);
      tempFCont = (char *) malloc(tempFSize); //Assign the file size to the string file content
      printf("File size: %s\n", tempFSizeS);
      recv(new_sockfd, tempFCont, tempFSize, 0); /* Receive file content from client */

      strcpy(tempPathName, "user_data/");
      strcat(tempPathName, tempFName);
      printf("Finished create path ... \nPath: %s\n", tempPathName);

      int j = 0;
      while(fileExists(tempPathName)){
        j++;
        printf("File Number: %d", j);
        char fileNumber[4]; //Temp string to store file number
        char tempMsg0[BUFSIZE + 1]; //Temp string to store message content
        char *tempPNE[2]; //To store path name and extension seperately
        int tempJ;
        while(tempJ < 2){
          tempPNE[tempJ] = (char *) malloc(BUFSIZE + 1);
          tempJ++;
        }
        strcpy(tempMsg0,tempPathName);
        char *tempFNameExt = strtok(tempMsg0, "."); //Temp string to store file extension
        int k = 0;
        while(tempFNameExt){
          strcpy(tempPNE[k], tempFNameExt);
          tempFNameExt = strtok(NULL, ".");
          k++;
        }
        sprintf(fileNumber, "%d", j); //Convert int j to string
        /** Combine the new name to become a legit path name **/
        strcat(tempPNE[0], fileNumber);
        strcat(tempPNE[0], ".");
        strcat(tempPNE[0], tempPNE[1]);
        strcpy(tempPathName, tempPNE[0]);
      }
      int tempFd;
      tempFd = creat (tempPathName, 00777);
      write(tempFd,tempFCont,tempFSize);
      printf("Message content: %s\n", tempFCont);
      printf("Finished uploaded... File is stored at: %s\n", tempPathName);
      send(new_sockfd, "Upload complete...", 20, 0); //Send success message to client
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

int getFileSize(FILE *fdo){
  int sz;
  fseek(fdo, 0L, SEEK_END);
  sz = ftell(fdo);
  fseek(fdo, 0L, SEEK_SET);
  return sz;
}

int fileExists(const char *fname){
    FILE *file;
    if (file = fopen(fname, "r")){
        fclose(file);
        return 1;
    }
    return 0;
}

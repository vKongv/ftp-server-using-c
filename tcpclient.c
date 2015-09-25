/* tcpclient.c */
/* Version 0.4
    Major update - User able to upload file to server's user_data directory
    Other changes:
    - User can download and upload any kind of file. (Before this, limit to 1024 bytes only)
    - Improve code readability
*/
#include "inet.h"
#define BUFSIZE 1024

int sockfd;
char buffer[BUFSIZE+1];
struct sockaddr_in serv_addr;

int errorChecking();
int getFileSize(FILE *);

int main (int argc, char *argv[]){

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

  /**
    Usage of msg
    1st type:
      -To store message status and message content [0] = status [1] = content
  **/
  char *msg[2];
  int tempI = 0;
  while(tempI < 2){
    msg[tempI] = (char *) malloc(BUFSIZE + 1);
    tempI++;
  }
  for(;;){
    recv(sockfd, buffer, BUFSIZE, 0); /* Receive message from server */
    char *tempMsg = strtok(buffer,"^");
    //printf("After strtok: %s\n", tempMsg);
    int i = 0;
      while(tempMsg){
        //printf("Transfering data to msg: %s\n", tempMsg);
        strcpy(msg[i], tempMsg);
        tempMsg = strtok(NULL, "^");
        i++;
      }
    printf("Server status: %s\n", msg[1]); //First message is to display
    printf("\n%s\n",msg[0]);
    //bzero (buffer, sizeof(buffer)); /* Clear all the data in the buffer */
    gets(buffer);
    //If the server status is DOWNLOADING, provide
    if((strcmp(msg[1], "SELECTING")) == 0){
      send (sockfd, buffer, BUFSIZE, 0); /* Send the data to server */
      bzero (buffer, sizeof(buffer)); /* Clear all the data in the buffer */
      recv(sockfd, buffer, BUFSIZE, 0); /* Receive file name from server */
      printf("Display 2nd message...\n"); //Second message is data
      //printf("Before strtok: %s\n", buffer);
      //char *tempString = strtok(buffer,"^");
      //printf("After strtok: %s\n", tempString);
    //  int i = 0;
        //while(tempString){
        //  printf("Transfering data to msg: %s\n", tempString);
          //msg[i] = tempString;
          //tempString = strtok(NULL, "");
          //i++;
        //}
        printf ("Message received from server : \n%s\n", buffer); // Print out the message received from server
      if((errorChecking()))
        break;
    }
    else if((strcmp(msg[1], "DOWNLOADING")) == 0){
      /**
      First receive file name
      Second receive file size
      Third receive file content
      **/
      //FILE *fd; // File descriptor to write new file
      int tempFSize; //File size
      char *tempFSizeS = (char *) malloc(40); //Char file size
      char *tempFName = (char *) malloc(BUFSIZE + 1); //File name
      char tempPathName[BUFSIZE + 1]; //Path name
      char *tempFCont; //File content
      send (sockfd, buffer, BUFSIZE, 0); /* Send the data to server */
      bzero (buffer, sizeof(buffer)); /* Clear all the data in the buffer */
      recv(sockfd, buffer, BUFSIZE, 0); /* Receive file name from server */
      strcpy(tempFName,buffer);
      printf("In downloading...\n");
      if(!(errorChecking())){
        recv(sockfd, tempFSizeS, 40, 0); /* Receive file size from server */
        tempFSize = atoi(tempFSizeS); // Convert the file size string to integer
        tempFCont = (char *) malloc(tempFSize); //Assign the file size to the string file content
        recv(sockfd, tempFCont, tempFSize, 0); /* Receive file content from server */
        /** Get user download directory file **/
        struct passwd *p = getpwuid(getuid());
        strcpy(tempPathName, "/home/");
        strcat(tempPathName, p->pw_name);
        strcat(tempPathName, "/Downloads/");
        /**Specify the download file name **/
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
        printf("Finished downloaded... File is stored at: %s\n", tempPathName);
      }
      else{
        printf("Server message: %s\n", buffer );
      }
    }

    else if ((strcmp(msg[1], "UPLOADING")) == 0){
      printf("Buffer is %s\n", buffer);
      int fd;
      int tempFSize; //File size
      char *tempFSizeS = (char *) malloc(40); //Char file size
      char *tempFName = (char *) malloc(BUFSIZE + 1); //File name
      char tempPathName[BUFSIZE + 1]; //Path name
      char *tempFCont; //File content

      strcpy(tempPathName, buffer);
      FILE *tempFile;
      //Check file exist or not
      printf("%s^\n",tempPathName);
      if ((tempFile = fopen (tempPathName, "r+")) == NULL)
        printf("File open fail");
      //To get file name from path name
      char *tempBuf = (char *) malloc(BUFSIZE + 1);
      char *tempBuf2 = (char *) malloc(BUFSIZE + 1);
      char *realFName = (char *) malloc(BUFSIZE + 1);
      strcpy(tempBuf,buffer);
      tempBuf2 = strtok(tempBuf, "/");
      while(tempBuf2){
        strcpy(realFName, tempBuf2);
        tempBuf2 = strtok(NULL, "/");
      }
      printf("Real file name is: %s\n", realFName);
      send(sockfd, realFName, BUFSIZE, 0); //Send file name to server
      printf("File name is: %s\n",realFName);
      tempFSize = getFileSize(tempFile);
      sprintf(tempFSizeS, "%d", tempFSize);
      send(sockfd,tempFSizeS,40,0); //Send file size to server
      fclose(tempFile);
      printf("Passed file size to server\n");
      printf("Path name is: %s\n", tempPathName);
      tempFCont = (char *) malloc(tempFSize); //Allocate file size to the content
      fd = open(tempPathName, 0);
      read (fd, tempFCont, tempFSize); //Read the file content into the buffer
      printf("File content: %s\n", tempFCont);
      send(sockfd, tempFCont, tempFSize,0);  //Send the file content to server
      close(fd);
      recv(sockfd,buffer,BUFSIZE,0);
      printf("Server: %s",buffer);
    }
  }
  close (sockfd); /* Close the connecting port */
  return 0;
}

int errorChecking(){
  if((strcmp(buffer, ERR_MSG[0]) == 0) || (strcmp(buffer, ERR_MSG[1]) == 0) || (strcmp(buffer, "Bye!\n") == 0))
    return 1;
  return 0;
}

int fileExists(const char *fname){
    FILE *file;
    if (file = fopen(fname, "r")){
        fclose(file);
        return 1;
    }
    return 0;
}

int getFileSize(FILE *fdo){
  int sz;
  fseek(fdo, 0L, SEEK_END);
  sz = ftell(fdo);
  fseek(fdo, 0L, SEEK_SET);
  return sz;
}

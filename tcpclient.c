/* tcpclient.c */
/* Version 1.0
    - Complete working version of TCP Client
    - Two major functions are working fine.
    - Some minor code changes
*/
#include "inet.h"
#define BUFSIZE 1024

int sockfd;
char buffer[BUFSIZE+1];
struct sockaddr_in serv_addr;

int errorChecking();
int getFileSize(FILE *);
int fileExists(const char *);
void quit(int);
int checkQuit(const char *);

int main (int argc, char *argv[]){

  if (argc <= 1){
    printf("How to use: %s remoteIPaddress [example: ./client 127.0.0.1]\n", argv[0]);
    exit(1);
  }

  static struct sigaction exitC;

  exitC.sa_handler = quit;

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

  sigaction(SIGINT, &exitC, (void *)0);
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
    if(checkQuit(buffer)){
      close(sockfd);
      bzero(buffer,sizeof(buffer));
      exit(0);
    }
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
    if((strcmp(msg[1], STATUS[4])) == 0){
      printf("Server: Crashed...\nExiting...\n");
      exit(1);
    }
    //bzero (buffer, sizeof(buffer)); /* Clear all the data in the buffer */
    gets(buffer);
    //If the server status is DOWNLOADING, provide
    if((strcmp(msg[1], STATUS[1])) == 0){
      send (sockfd, buffer, BUFSIZE, 0); /* Send the data to server */
      bzero (buffer, sizeof(buffer)); /* Clear all the data in the buffer */
      recv(sockfd, buffer, BUFSIZE, 0); /* Receive file name from server */
      if(checkQuit(buffer)){
        bzero(buffer,sizeof(buffer));
        close(sockfd);
        exit(0);
      }
        printf ("Message received from server : \n%s\n", buffer); // Print out the message received from server
      if((errorChecking()))
        break;
    }
    else if((strcmp(msg[1], STATUS[2])) == 0){
      /**
      First receive file name
      Second receive file size
      Third receive file content
      **/
      //FILE *fd; // File descriptor to write new file
      if((strcmp(buffer,"/q")) == 0){
        system("clear");
        send(sockfd,buffer,BUFSIZE,0);
        continue;
      }
      int tempFSize; //File size
      char *tempFSizeS = (char *) malloc(40); //Char file size
      char *tempFName = (char *) malloc(BUFSIZE + 1); //File name
      char tempPathName[BUFSIZE + 1]; //Path name
      char *tempFCont; //File content
      send (sockfd, buffer, BUFSIZE, 0); /* Send the data to server */
      bzero (buffer, sizeof(buffer)); /* Clear all the data in the buffer */
      recv(sockfd, buffer, BUFSIZE, 0); /* Receive file name from server */
      if(checkQuit(buffer)){
        bzero(buffer,sizeof(buffer));
        close(sockfd);
        exit(0);
      }

      strcpy(tempFName,buffer);
      printf("File name: %s\n", tempFName);
      printf("In downloading...\n");
      if(!(errorChecking())){
        recv(sockfd, tempFSizeS, 40, 0); /* Receive file size from server */
        if(checkQuit(tempFSizeS)){
          close(sockfd);
          bzero(tempFSizeS,sizeof(tempFSizeS));
          exit(0);
        }
        printf("File size: %s\n", tempFSizeS);
        tempFSize = atoi(tempFSizeS); // Convert the file size string to integer
        tempFCont = (char *) malloc(tempFSize); //Assign the file size to the string file content
        recv(sockfd, tempFCont, tempFSize, 0); /* Receive file content from server */
        if(checkQuit(tempFCont)){
          bzero(tempFCont,sizeof(tempFCont));
          close(sockfd);
          exit(0);
        }
        /** Get user download directory file **/
        struct passwd *p = getpwuid(getuid());
        strcpy(tempPathName, "/home/");
        strcat(tempPathName, p->pw_name);
        strcat(tempPathName, "/Downloads/");
        /**Specify the download file name **/
        strcat(tempPathName, tempFName);
        printf("Finished create path ... \nPath: %s\n", tempPathName);
        int j = 0;
        char *oriPathName = (char *) malloc(BUFSIZE + 1);
        strcpy(oriPathName, tempPathName);
        while(fileExists(tempPathName)){
          j++;
          char fileNumber[4]; //Temp string to store file number
          char tempMsg0[BUFSIZE + 1]; //Temp string to store message content
          char *tempPNE[2]; //To store path name and extension seperately
          int tempJ = 0;
          while(tempJ < 2){
            tempPNE[tempJ] = (char *) malloc(BUFSIZE + 1);
            tempJ++;
          }
          strcpy(tempMsg0,oriPathName);
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
        send(sockfd, "OK", BUFSIZE, 0);
        printf("\n****************************SUCCESSFUL DOWNLOAD****************************\n");
      }
      else{
        printf("Server message: %s\n", buffer );
      }
    }

    else if ((strcmp(msg[1], STATUS[3])) == 0){
      printf("Buffer is %s\n", buffer);
      int fd;
      int tempFSize; //File size
      char *tempFSizeS = (char *) malloc(40); //Char file size
      char *tempFName = (char *) malloc(BUFSIZE + 1); //File name
      char tempPathName[BUFSIZE + 1]; //Path name
      char *tempFCont; //File content
      if((strcmp(buffer,"/q")) == 0){
        system("clear");
        send(sockfd,buffer,BUFSIZE,0);
        continue;
      }
      strcpy(tempPathName, buffer);
      FILE *tempFile;
      //Check file exist or not
      if ((tempFile = fopen (tempPathName, "r+")) == NULL){
        send(sockfd, ERR_MSG[0],25,0);
        printf("File open fail\n");
        continue;
      }
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
      if(checkQuit(buffer)){
        bzero(buffer,sizeof(buffer));
        close(sockfd);
        exit(0);
      }
      if((strcmp(buffer,"OK")) == 0){
        printf("\n****************************SUCCESSFUL UPLOAD****************************\n");
      }
      else{
        printf("\n****************************FAIL UPLOAD****************************\n");
      }
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
    int fd;
    if ((fd = open(fname, 0)) >= 0){
        close(fd);
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

void quit(int sig){
  printf("Quit now...\n");
  send(sockfd, ERR_MSG[3], 20, 0);
  close(sockfd);
  exit(0);
}

int checkQuit(const char *tempStr){
  if ((strcmp(tempStr, ERR_MSG[3])) == 0){
    return 1;
  }
  else
    return 0;
}

/* tcpclient.c */
/* Version 0.3
    Major update - User able to download files from server
*/
#include "inet.h"
#define BUFSIZE 1024

int sockfd;
char buffer[BUFSIZE+1];
struct sockaddr_in serv_addr;

int errorChecking();

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
    Two types of usage:
    1st type:
      -To store message status and message content [0] = status [1] = content
    2nd type:
      -To store filename and file content [0] = filename [1] = content
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
    bzero (buffer, sizeof(buffer)); /* Clear all the data in the buffer */
    gets(buffer); /* Get input from user*/
    send (sockfd, buffer, BUFSIZE, 0); /* Send the data to server */
    bzero (buffer, sizeof(buffer)); /* Clear all the data in the buffer */
    recv(sockfd, buffer, BUFSIZE, 0); /* Receive message from server */
    //If the server status is DOWNLOADING, provide
    if((strcmp(msg[1], "DOWNLOADING")) == 0){
      printf("In downloading...\n");
      if(!(errorChecking())){
        int fd; // File descriptor to write new file
        char *tempMsg2 = (char *) malloc(BUFSIZE + 1);
        strcpy(tempMsg2, buffer);
        tempMsg2 = strtok(tempMsg2,"#");
        int j = 0;
        while(tempMsg2){
          strcpy(msg[j], tempMsg2);
          tempMsg2 = strtok(NULL, "#");
          j++;
        }
        printf("Finished strtok #1...\n");
        /** Get user download directory file **/
        struct passwd *p = getpwuid(getuid());
        char tempPathName[BUFSIZE + 1] = "/home/";
        strcat(tempPathName, p->pw_name);
        strcat(tempPathName, "/Downloads/");
        /**Specify the download file name **/
        strcat(tempPathName, msg[0]);
        strcpy(msg[0], tempPathName);
        printf("Finished create path ... \nPath: %s\n", msg[0]);
        int k = 0;
        while(fileExists(msg[0])){
          k++;
          printf("File Number: %d", k);
          char fileNumber[4]; //Temp string to store file number
          char tempMsg0[BUFSIZE + 1]; //Temp string to store message content
          char *tempPNE[2]; //To store path name and extension seperately
          int tempJ;
          while(tempJ < 2){
            tempPNE[tempJ] = (char *) malloc(BUFSIZE + 1);
            tempJ++;
          }
          strcpy(tempMsg0,msg[0]);
          char *tempFileNameExt = strtok(tempMsg0, "."); //Temp string to store file extension
          int l = 0;
          while(tempFileNameExt){
            strcpy(tempPNE[l], tempFileNameExt);
            tempFileNameExt = strtok(NULL, "#");
            l++;
          }
          sprintf(fileNumber, "%d", k); //Convert int k to string
          /** Combine the new name to become a legit path name **/
          strcat(tempPNE[0], fileNumber);
          strcat(tempPNE[0], ".");
          strcat(tempPNE[0], tempPNE[1]);
          strcpy(msg[0], tempPNE[0]);
        }
        fd = creat (msg[0], 00777);
        write(fd,msg[1],sizeof(msg[1]));
        printf("Finished downloaded... File is stored at: %s\n", msg[0]);
      }
      else{
        printf("Server message: %s\n", buffer );
      }
    }
    else{
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

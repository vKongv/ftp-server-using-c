/* tcpserver.c */
/* Version 1.0
    - Complete working version of TCP server
    - Able to dynamically retrieve IP address of server. (Before is static 192.168.1.253, user might need to change the IP address of server to suit this)
    - Able to show number of active client and IP address of new client (Using pipe and signal)
    - Two major functions are working fine.
*/
#include "inet.h"


/**
  Tokenizer list:
    ^ - To combine STATUS of server
    $ - To combine DIRECTORY list
**/
/**
  connectedClient = To store number of connected client
  childpid = To store child pid
  sockfd = Listening socket
  new_sockfd = Processing socket
  clilen = Client address's length
  yes = To be used in address reuse function
  servIP = To store server's IP address
  buffer = To store data
  directory = To store path to directory.txt
  serv_addr = Structure of sockaddr_in of Server IP address
  cli_addr = Structure of sockaddr_in of Client IP address
  st = To check status of the user_data directory
  currentStatus = Current status of server
  p1 = Pipe for parent and child communication
**/
int connectedClient = 0;
int childpid;
int childStatus = 0;
int sockfd, new_sockfd, clilen;
int yes = 1;
char * confirmation;
char *servIP;
char *buffer;
//char displayMessage[BUFSIZE + 1];
char directory[BUFSIZE + 1];
struct sockaddr_in serv_addr, cli_addr;
struct stat st = {0};
char *currentStatus;
int p1[2]; //p1[0] is for reading, p1[1] is for writing

/**
  loadUserDirectory() = To reload user directory (After upload and download)
    Return type: int (-1 = fail to load)
    Paremeter: No paremeter
  sendInitialMessage() = To send initial message to client (Main Menu + Server status)
    Return type: void
    Paremeter: No paremeter
  getFileSize()
    Return type: int (-1 = fail to load)
    Paremeter: FILE (The file that had been open)
  fileExists() = To check whethere the file is exist or not
    Return type: int (0 = Not exist, 1 = Exist)
    Paremeter: char * (The path to the file)
  countClient() = To increase the counter of connectedClient (After child send SIGUSR1 signal to parent)
    Return type: void
    Paremeter: int (Signal type)
  checkQuit() = To check whethere client issue CRTL-C
    Return type: int (1 = want quit, 0 = Nothing)
    Paremeter: char * (The received buffer from user)
**/
int loadUserDirectory();
void sendInitialMessage();
int getFileSize(FILE *);
int fileExists(const char *);
void countClient(int);
void disClient(int);
int checkQuit(const char *, int);

int main () {
  buffer = (char *)malloc(BUFSIZE + 1);
  confirmation = (char *) malloc (CONSIZE + 1);
  /**
      SIGUSR1 = When client connected, issue this Signal
      SIGUSR2 = When client disconnected, issue this Signal
  **/
  static struct sigaction pact_c, pact_d;
  pact_c.sa_handler = countClient;
  pact_d.sa_handler = disClient;
  sigaction(SIGUSR1, &pact_c, (void *) 0);
  sigaction(SIGUSR2, &pact_d, (void *) 0);

  //Pipe to allow child to pass client's IP address
  if((pipe(p1)) == -1){
    perror("Server: pipe() error\n");
    exit(1);
  }

  //Allocate buffer for char *
  servIP =  (char *) malloc(20);
  currentStatus = (char *) malloc(20);
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

  //To execute the program retrieveIP, this is to get the IP address of server
  printf("\nTCP Server\n");
  if( (childpid = fork()) == -1){
    perror("Fork() got problem\nCannot start server\n");
    exit(1);
  }
  else if(childpid == 0){
    printf("Retrieving IP address...\n");
    if((execl("retrieveip", "retrieveip", (char *) 0)) == -1){
      exit(1);
    }
		exit(0);
  }
  wait(childStatus);
  if(childStatus != 0){
    printf("tcpserver: Fail to retrieve IP address...\n");
    exit(1);
  }
  //To receive the IP address stored in message queue. (The program will store it into a message queue for server to retrieve it IP address)
  if((receive(&servIP,5)) < 0){
    printf("tcpserver: Fail to get message from queue...\n");
    exit(2);
  }
  //Clear or delete the message queue
  clearmq();

  bzero(&serv_addr, sizeof(serv_addr)); /* Clear all the data in serv_addr */
  serv_addr.sin_family = AF_INET; /* It is an IPv4 address*/
  serv_addr.sin_addr.s_addr = inet_addr(servIP); /* Local IP address */
  printf("Server address is: %s\n", inet_ntoa(serv_addr.sin_addr));
  serv_addr.sin_port = htons(SERV_TCP_PORT); //Assign a local port to the address, in this case "25000".

  //Add local protocol address to a socket
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
    perror("Server: bind() error \n");
    exit(1);
  }
  printf("\nWaiting for connection... [bind]\n");
  listen(sockfd, 5); /* Listen to a connection, Max 5 queues */

  //Concurrent server start here
  for(;;){
  //printf("In Concurrent\n");
  //sleep(5);
  strcpy(currentStatus, STATUS[0]);
  clilen = sizeof(cli_addr); /* Get the size of client address */
  if ((new_sockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1){
    continue;
  } /* Accept the connection and assign a new socket to handle it*/

  if(fork() == 0){
    int tempPID = getppid();
    close(sockfd); //Close listening sockfd for child

    //Send new client message to parent
    write(p1[1], inet_ntoa(cli_addr.sin_addr),BUFSIZE); //Write IP address into pipe
    kill(tempPID, SIGUSR1); //Send the signal SIGUSR1 to parent

    if(new_sockfd > 0){
      printf("\nClient connected now.\n");
    }
    //Status = SELECTING
    strcpy(currentStatus, STATUS[1]);
    strcpy(directory, "user_data/directory.txt"); //For future use. (After implement user login function, each user can store in its own directory)

    //Main functions start here
    for(;;){
      //Load directory.txt to display for user
      if (loadUserDirectory() < 0){
        strcpy(currentStatus, STATUS[4]);
        strcpy(buffer, "Cannot load directory.../n");
        sendInitialMessage();
        exit(1);
      }

      int fd,fddir, selection = -1;
      //Main page selection
      if((strcmp(currentStatus, STATUS[1])) == 0){
        strcpy(buffer, "1. Download file \n2. Upload file \n3. Quit\nYour selection: ");
        sendInitialMessage(); // Send initial message
        printf("\n****************************In Main Menu****************************\n");
        recv(new_sockfd,buffer,BUFSIZE,0); /* Receive message from client */
        send(new_sockfd, REPLYCONF, CONSIZE + 1, 0);
        if(checkQuit(buffer, tempPID)){
          close(new_sockfd);
          //bzero(buffer,sizeof(buffer));
          printf("Preparing to exit...\n");
          exit(0);
        }
        printf("\nReceived message [%s] from CLIENT\n", buffer);
        /**
          1 - Read file
          2 - Write file
        **/
        selection = atoi(buffer);
        switch(selection){
          case 1:
            strcpy(currentStatus, STATUS[2]);
            send(new_sockfd, "OK", 2,0);  /* Send an OK message to client */
            recv(new_sockfd, confirmation, CONSIZE + 1, 0);
            continue;
          case 2:
            strcpy(currentStatus, STATUS[3]);
            send(new_sockfd, "OK", 2,0);  /* Send an OK message to client */
            recv(new_sockfd, confirmation, CONSIZE + 1, 0);
            continue;
          case 3:
            send(new_sockfd, "Bye!\n", 6,0); /* Send an OK message to client */
            checkQuit(ERR_MSG[3],tempPID);
            close(new_sockfd);  /* Close the processing socket */
            exit(0);
          default:
            strcpy(currentStatus, STATUS[1]);
            send(new_sockfd, "Invalid selection!\n", 20,0);  /* Send error message to client */
            recv(new_sockfd, confirmation, CONSIZE + 1, 0);
            continue;
        }
      }
      //Downlaod function
      else if ((strcmp(currentStatus, STATUS[2])) == 0){
        char *temptemp;
        temptemp = (char *) malloc(BUFSIZE + 1);
        int tempFSize; //File size
        char *tempFSizeS = (char *) malloc(40); //Char file size
        char *tempFName = (char *) malloc(BUFSIZE + 1); //File name
        char tempPathName[BUFSIZE + 1]; //Path name
        char *tempFCont; //File content

        if((fddir = open(directory,0)) < 0){
          perror("Cannot open directory.txt.\n");
          exit(1);
        }
        read(fddir,temptemp,BUFSIZE);
        char *tempBuf = (char *) malloc(BUFSIZE + 1);
        char *tempTokBuf = (char *) malloc(BUFSIZE + 1);
        strcpy(tempBuf,temptemp);
        tempTokBuf = strtok(tempBuf,"$");
        strcpy(temptemp, "");
        while(tempTokBuf){
          strcat(temptemp, tempTokBuf);
          strcat(temptemp, "   ");
          tempTokBuf = strtok(NULL, "$");
        }
        close(fddir);
        //Compiling initial message
        char tempMsg[BUFSIZE + 1];
        strcpy(tempMsg, temptemp);
        strcat(tempMsg,"\n");
        strcpy(buffer, "Please enter the file NAME you want to the SERVER [type /q to go to previous page]\n\nChoose from the listed file(s):\n\n");
        strcat(buffer, tempMsg);
        sendInitialMessage();
        printf("\n****************************Displaying Downlaod page****************************\n");
        recv(new_sockfd,buffer,BUFSIZE,0); /* Receive message from client */
        send(new_sockfd, REPLYCONF, CONSIZE + 1, 0);
        if(checkQuit(buffer, tempPID)){
          close(new_sockfd);
          bzero(buffer,sizeof(buffer));
          exit(0);
        }
        //If the incoming request is "/q" means user want to go back to selection
        if((strcmp(buffer, "/q") == 0)){
          strcpy(currentStatus, STATUS[1]);
          continue;
        }
        printf("\nReceived message [%s] from CLIENT\n", buffer);
        //Open the file that specific by client
        char tempDir[BUFSIZE+1] = "user_data/";
        strcat(tempDir,buffer);

        FILE *tempFile;
        if ((tempFile = fopen (tempDir, "r+")) == NULL ){
          send(new_sockfd, ERR_MSG[0], strlen(ERR_MSG[0]) + 1, 0);
          recv(new_sockfd, confirmation, CONSIZE + 1, 0);
          printf("\n****************************FAIL DOWNLOAD****************************\n");
          continue;
        }

        strcpy(tempFName, buffer);
        printf("File name is: %s\n", tempFName);
        send(new_sockfd, tempFName, strlen(tempFName) + 1,0);
        recv(new_sockfd, confirmation, CONSIZE + 1, 0);
        //sleep(5);
        tempFSize = getFileSize(tempFile);
        fclose(tempFile);
        fd = open(tempDir, 0);
        printf("File size is: %d bytes\n ", tempFSize);
        sprintf(tempFSizeS, "%d", tempFSize); //Convert file size to string
        send(new_sockfd, tempFSizeS, strlen(tempFSizeS) + 1,0);  /* Send the file size to client */
        recv(new_sockfd, confirmation, CONSIZE + 1, 0);
        //sleep(1);
        tempFCont = (char *) malloc(tempFSize); //Allocate file size to the content
        read (fd, tempFCont, tempFSize); //Read the file content into the buffer
        send(new_sockfd, tempFCont, tempFSize,0);  /* Send the file content to client */
        recv(new_sockfd, confirmation, CONSIZE + 1, 0);
        //sleep(1);
        close(fd);
        recv(new_sockfd,buffer,BUFSIZE,0);
        send(new_sockfd, REPLYCONF, CONSIZE + 1, 0);
        if(checkQuit(buffer, tempPID)){
          close(new_sockfd);
          bzero(buffer,sizeof(buffer));
          exit(0);
        }
        if((strcmp(buffer,"OK")) == 0){
          printf("\n****************************SUCCESSFUL DOWNLOAD****************************\n");
        }
        else{
          printf("\n****************************FAIL DOWNLOAD****************************\n");
        }
      }
      //Upload function
      else if ((strcmp(currentStatus, STATUS[3])) == 0){
        int tempFSize; //File size
        char *tempFSizeS = (char *) malloc(40); //Char file size
        char *tempFName = (char *) malloc(BUFSIZE + 1); //File name
        char tempPathName[BUFSIZE + 1]; //Path name
        char *tempFCont; //File content

        strcpy(buffer, "Please enter the COMPLETE path of your file that you want to upload: [type /q to go to previous page]\n");
        sendInitialMessage();
        printf("\n****************************Displaying Upload page****************************\n");
        bzero(buffer,sizeof(buffer));
        recv(new_sockfd, buffer, BUFSIZE, 0); /* Receive file name from client */
        send(new_sockfd, REPLYCONF, CONSIZE + 1, 0);
        if(checkQuit(buffer, tempPID)){
          close(new_sockfd);
          bzero(buffer,sizeof(buffer));
          exit(0);
        }
        if((strcmp(buffer,ERR_MSG[0])) == 0){
          printf("Client: Cannot open file...\n");
          continue;
        }
        //If the incoming request is "/q" means user want to go back to selection
        if((strcmp(buffer, "/q") == 0)){
          strcpy(currentStatus, STATUS[1]);
          continue;
        }
        printf("Receveid file name: %s\n", buffer);
        strcpy(tempFName,buffer);
        recv(new_sockfd, tempFSizeS, 40, 0); /* Receive file size from client */
        send(new_sockfd, REPLYCONF, CONSIZE + 1, 0);
        if(checkQuit(tempFSizeS, tempPID)){
          close(new_sockfd);
          bzero(tempFSizeS,sizeof(tempFSizeS));
          exit(0);
        }
        tempFSize = atoi (tempFSizeS);
        tempFCont = (char *) malloc(tempFSize); //Assign the file size to the string file content
        printf("File size: %s bytes\n ", tempFSizeS);
        strcpy(tempPathName, "user_data/");
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
        int sumByte = 0;
        tempFd = creat (tempPathName, 00777);
        while(1){
          int temp;
          lseek(tempFd,0,SEEK_END);
          temp = recv(new_sockfd, tempFCont, tempFSize , 0);
          write(tempFd,tempFCont,temp);
          sumByte += temp;
          if(sumByte == tempFSize){
            break;
          }
        }
        printf("Message content: %s\n", tempFCont);
        send(new_sockfd, REPLYCONF, CONSIZE + 1, 0);
        printf("Finished uploaded... File is stored at: %s\n", tempPathName);
        send(new_sockfd, "OK", 3, 0); //Send success message to client
        recv(new_sockfd, confirmation, CONSIZE + 1, 0);
        printf("\n****************************SUCCESSFUL UPLOAD****************************\n");
      }
      //Unknow server error
      else{
        printf("Unknown server error!\n");
        strcpy(currentStatus, STATUS[4]);
        strcpy(buffer, "Unknown server error\n");
        sendInitialMessage();
        exit(1);
      }
    }
      exit(0);
  }
  else{
    pause(); //Wait for signal from child
    printf("After kill...\n");
    //sleep(5);
  }
  close(new_sockfd);  /* Close the processing socket */
}
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
  int temp = send(new_sockfd, currentStatus, strlen(currentStatus) + 1,0);  /* Send the initial message to client */
  temp = recv(new_sockfd, confirmation, CONSIZE + 1, 0);
  temp = send(new_sockfd, buffer, strlen(buffer) + 1,0);  /* Send the initial message to client */
  temp = recv(new_sockfd, confirmation, CONSIZE + 1, 0);
}

int getFileSize(FILE *fdo){
  int sz;
  fseek(fdo, 0L, SEEK_END);
  sz = ftell(fdo);
  fseek(fdo, 0L, SEEK_SET);
  return sz;
}

int fileExists(const char *fname){
    int fd;
    if ((fd = open(fname, 0)) >= 0){
        close(fd);
        return 1;
    }
    return 0;
}

void countClient(int sig){
  char tempIP[BUFSIZE + 1];
  if((read(p1[0], tempIP, BUFSIZE)) <= 0){
    perror("Server: Read pipe fail...\n");
    exit(1);
  }
  connectedClient ++;
  printf("\n****************************NEW CLIENT****************************\n");
  printf("New client connected!\nNo. Active Client: %d\n", connectedClient);
  printf("New client IP address: %s\n", tempIP);
}

int checkQuit(const char *tempStr, int parentID){
  if ((strcmp(tempStr, ERR_MSG[3])) == 0){
    write(p1[1], inet_ntoa(cli_addr.sin_addr),BUFSIZE); //Write IP address into pipe
    kill(parentID, SIGUSR2);
    return 1;
  }
  else
    return 0;
}

void disClient(int sig){
  bzero(&cli_addr, sizeof(cli_addr));
  char tempIP[BUFSIZE + 1];
  if((read(p1[0], tempIP, BUFSIZE)) <= 0){
    perror("Server: Read pipe fail...\n");
    exit(1);
  }
  connectedClient --;
  printf("\n****************************CLIENT DISCONNECTED****************************\n");
  printf("No. Active Client: %d\n", connectedClient);
  printf("Disconnected client IP address: %s\n", tempIP );
}

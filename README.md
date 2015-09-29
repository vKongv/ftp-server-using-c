# ftp-server-using-c
# BEFORE RUN THE PROGRAM
- Download all the files (6 files) in this respository (tcpserver.c, tcpclient.c, inet.h, queue.h, qoperate.h, retrieveip.c)
- Put all the files in the same directory
- Edit the retrieveip.c line no. 11 if you are using different connection (Default is eth0), Use ifconfig to check.
- Compile all the C files using gcc filename -o outfilename (Please use the same file name for the output file)
- FINAL CHECKLIST: You should have the 3 executable files (tcpserver, tcpclient, retrieveip)
- FOR BEST RESULT, PLEASE TEST BOTH CLIENT AND SERVER PROGRAM IN ONE MACHINE!!!

#RUN THE PROGRAM
- Run the server first (./tcpserver)
- Then run the client in another terminal (./tcpclient ipaddressOfServer)
- Perform the task you wanted.
- PLEASE TEST THE PROGRAM IN LINUX AND ON THE SAME MACHINE, THERE ARE SOME BUGS WHEN DOING IN DIFERENT MACHINES

#FEATURES
- Able to download and upload file
- Concurrent servers (Handle multiple clients at one time)
- Display connected client to the server terminal (When new client connect to the server)
- Dynamically retrieve server IP address (Must configure to the correct interface in the C file before compilation)
- Handle CRTL-C from client (Disconnect it from server)

#LIMITATION
- ONLY can run in LINUX OS (Mac OS will need to edit the download path)
- ONLY can run in the SAME machine! (Run in other machine will have some data transmission delay problem that cause serious bug in the program)
- ONLY can run in different machine when sending TEXT files
- CANNOT delete file(s)

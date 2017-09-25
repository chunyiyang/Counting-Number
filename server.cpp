#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <queue>
#include <iostream>
#include <strstream>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctime>


using namespace std;

#define PORT 9740 
#define BUF_SIZE 200



int main()
{
    int socket_desc, client_sock, c;
    struct sockaddr_in server, client;
    char number[BUF_SIZE];
    int rv;
    int ran_num;    
    
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    cout << "Socket created" << endl;

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( PORT );

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return -1;
    }
    cout << "Bind success" << endl;

    //Listen
    listen(socket_desc , 3);

    //Accept and incoming connection
    cout << "Waiting for incoming connections..." << endl;
    c = sizeof(struct sockaddr_in);

    //accept connection from an incoming client
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0)
    {
        perror("accept failed");
        return -1;
    }
    cout << "Connection success" << endl;

    while (1) {
      srand((unsigned)clock());
      ran_num=rand() % 65535;
      sprintf(number, "%d", ran_num);
      strcat(number, "\n");
      cout << "send :" << number << endl;
      rv = write(client_sock , number , strlen(number));
      if(rv == 0)
      {
          cout << "Client disconnected" << endl;
          fflush(stdout);
          break;
      }
      else if(rv == -1)
      {
          perror("recv failed");
          break;
      }
      usleep(5000);
    }
    return 0;
}

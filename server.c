#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#define MAXDATASIZE 1000

int main(int argc, char const *argv[])
{
	char *port = "3000";

  struct addrinfo hints,*res,*p;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

	int status;

	if ((status = getaddrinfo(NULL, (const char*)port, &hints, &res)) != 0)
  {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
      return 2;
	}

  int sockfd;
  if ((sockfd = socket(res->ai_family, res->ai_socktype,res->ai_protocol)) == -1)
  {
      perror("client: socket");
  }

  if(bind(sockfd, res->ai_addr, res->ai_addrlen) < 0)
  {
    perror("Server: socket");
  }

  if(listen(sockfd,5) < 0)
  {
    perror("Listen error");
  }

  while(1)
    {
        struct sockaddr_in client_address;
        socklen_t len = sizeof(client_address);

        int newsock;
        if((newsock = accept(sockfd, (struct sockaddr *)&client_address, &len)) < 0)
        {
          perror("Accept error");
          exit(1);
        }

        char ip[INET6_ADDRSTRLEN];  // space to hold the IPv4 string
        if(client_address.sin_family == AF_INET)
        {
          inet_ntop(AF_INET, &(client_address.sin_addr), ip, INET_ADDRSTRLEN); // IPv4 network to presentation
        }
        else
        {
          inet_ntop(AF_INET6, &(client_address.sin_addr), ip, INET6_ADDRSTRLEN); // IPv6 network to presentation
        }

        printf("Received connection from %s\n",ip);

        while(1)
        {
          int i;
          char buffer[200]={0};
          if((i = read(newsock, buffer, sizeof(buffer))) < 0)
          {
              perror("Receive error");
              break;
          }

          if(i == 0)
          {
              printf("Socket closed remotely\n");
              break;
          }
          if(i > 0)
          {
              printf("received %d bytes\n",i);
              buffer[i] = '\0';
              printf("data: %s\n",buffer);

              char msg[] = "Got it! ";
              write(newsock, msg, strlen(msg));
          }
        }

        close(newsock);
    }
    close(sockfd);

}

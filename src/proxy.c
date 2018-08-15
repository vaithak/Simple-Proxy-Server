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
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "../includes/proxy_parse.h"

#define MAXDATASIZE 20001

void shiftMsg(char* start,int positions)
{
  int n = strlen(start);
  for(int j=0;j<n-positions;j++)
  {
    start[j]=start[j+positions];
  }
  start[n-positions]='\0';
}

// SEND GET REQUEST
int sendRequest(struct ParsedRequest *req, char *host, char response[], int response_len, int newsock)
{
  // printf("%s\n",req->protocol );
  req->protocol = "";
  req->host = "";
  // req->path;
  struct addrinfo hints,*res;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
  hints.ai_socktype = SOCK_STREAM;
  int status;
	if ((status = getaddrinfo(host, "80", &hints, &res)) != 0)
  {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
      return -1;
	}

  int sockfd;
  if ((sockfd = socket(res->ai_family, res->ai_socktype,res->ai_protocol)) == -1)
  {
      perror("client: socket");
  }
  // printf("%s\n",host);
  if(connect(sockfd, res->ai_addr, res->ai_addrlen) != -1)
  {
  	// convert the IP to a string and print it:
  	char ipstr[INET6_ADDRSTRLEN];
  	void *addr;
  	if (res->ai_family == AF_INET)
		{ // IPv4
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
      addr = &(ipv4->sin_addr);
    }
    else
    { // IPv6
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)res->ai_addr;
      addr = &(ipv6->sin6_addr);
		}

  	inet_ntop(res->ai_family, addr, ipstr, sizeof(ipstr));
  }
  else
  {
    shutdown(sockfd,2);
    perror("client: connect");
  	return -1;
  }

  freeaddrinfo(res);

  int len = ParsedRequest_totalLen(req);
  char msg_send[len+1];
  if(ParsedRequest_unparse(req, msg_send, len) < 0)
  {
    printf("parse ffailed\n");
    return -1;
  }
  msg_send[len]='\0';
  shiftMsg(msg_send+4,3);
  // printf("%s\n", msg_send);
  len = strlen(msg_send);
  int bytes_sent = send(sockfd, msg_send, len+1, 0);
  printf("Bytes Sent: %d\n",bytes_sent );

  int numbytes ;
  if((numbytes = recv(sockfd, response, response_len, 0)) == -1)
  {
    perror("recv");
    return -1;
  }

  do{
    printf("%s\n",response);
    numbytes =recv(sockfd,response,response_len,0);
    response[numbytes] = '\0';
    if(response[numbytes-4]=='\r' && response[numbytes-3]=='\n' && response[numbytes-2] == '\r' && response[numbytes-1]=='\n')
      break;

    write(newsock, response, strlen(response));
  }while(numbytes > 0);

  response[numbytes] = '\0';

  return close(sockfd);

}


// SIGCHLD handler
void my_sigchld_handler(int s)
{
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

// Handling HTTP Request
int handleRequest(int newsock, struct sockaddr_in client_address)
{
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
    char request[2000]={0};
    if((i = read(newsock, request, sizeof(request))) < 0)
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
        request[i] = '\0';

        // const char *c =
        // "GET http://www.google.com/ HTTP/1.0"
        // "\r\nIf-Modified-Since: Sat, 29 Oct 1994 19:43:31 GMT\r\n\r\n";
        printf("data: %s\n",request);
        // printf("data: %s\n",c);
        struct ParsedRequest *req = ParsedRequest_create();
        if (ParsedRequest_parse(req, request, strlen(request)) < 0) {
          char *errorRes = "400 Bad Request";
          write(newsock, errorRes, strlen(errorRes));
          ParsedRequest_destroy(req);
          return close(newsock);
        }

        char msg[MAXDATASIZE];
        if( strcmp(req->method, "GET") == 0 )
        {
          ParsedHeader_set(req,"Connection","close");
          if(sendRequest(req,req->host,msg,MAXDATASIZE,newsock) != -1)
            continue;
            // write(newsock, msg, strlen(msg));
        }
        else
        {
          char *errorRes = "500 Internal Server Error(Only Get is Implemented)";
          write(newsock, errorRes, strlen(errorRes));
          ParsedRequest_destroy(req);
          return close(newsock);
        }

        ParsedRequest_destroy(req);
    }
  }

  return close(newsock);
}


// Main function
int main(int argc, char const *argv[])
{
	char *port = "3000";

  if(argc == 2)
		port = (char*)argv[1];

  struct addrinfo hints,*res;
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

  printf("Proxy Server listening on port:%s\n", port);
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = my_sigchld_handler;
  sa.sa_flags = SA_RESTART; // Restart signal handler if interrupted
  sigaction(SIGCHLD, &sa, NULL);

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

        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
            handleRequest(newsock,client_address);
            exit(0);
        }
        close(newsock);  // parent doesn't need this
    }
    close(sockfd);

}

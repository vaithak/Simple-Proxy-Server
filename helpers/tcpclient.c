#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>

#define MAXDATASIZE 1000

int main(int argc, char const *argv[])
{
	if(argc < 2)
	{
		fprintf(stderr ,"Invalid Command! Usage: tcpclient hostname <port_no>\n");
		return 1;
	}

	// To measure RTT
	clock_t start = clock();

	char *port = "80";
	if(argc == 3)
		port = (char*)argv[2];

	struct addrinfo hints,*res,*p;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;

	int status;

	if ((status = getaddrinfo(argv[1], (const char*)port, &hints, &res)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
	}

	int sockfd ;//= socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	// loop through all the results and connect to the first we can
    for(p = res; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1)
        {
	            perror("client: socket");
				continue;
		}

		break;
	}

	if (sockfd == -1)
	{
        perror("client: socket");
        return 1;
    }

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
    	printf("Connection Setup with: %s (%s)\n", argv[1], ipstr);
    	printf("Press Ctrl + C to terminate the connection\n");
    }
    else
    {
    	shutdown(sockfd,2);
        perror("client: connect");
		return 1;
    }

    freeaddrinfo(res);

		// To measure RTT
		clock_t end = clock();
		printf("\nConnection Setup Time: %lf ms",100*((double)(end-start))/CLOCKS_PER_SEC);

    while(1)
    {
    	printf("\nEnter the string you want to send, end your string with * : ");
    	char msg_send[80];// Maximum length is 80 characters for the message to send
    	scanf("%[^*]s",msg_send);
    	int len = strlen(msg_send);

			start = clock();
    	int bytes_sent = send(sockfd, msg_send, len, 0);
    	printf("Bytes Sent: %d\n",bytes_sent );
    	char msg_received[MAXDATASIZE];
    	int numbytes ;
    	if((numbytes = recv(sockfd, msg_received, MAXDATASIZE-1, 0)) == -1)
    	{
    		perror("recv");
				return 1;
    	}

    	printf("Bytes Received: %d\n",numbytes );
    	msg_received[numbytes] = '\0';
	    printf("Client Received:\n%s\n",msg_received);

			end = clock();
			printf("\nRound Trip Time: %lf ms",100*((double)(end-start))/CLOCKS_PER_SEC);

	    while ((getchar()) != '*');
    }

    printf("Closing the connection ...\n");
    shutdown(sockfd,-2);

	return 0;
}

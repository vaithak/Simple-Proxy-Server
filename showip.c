/*
=> Important things for socket programming in C

struct addrinfo
{
    int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
    int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
    int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
    int              ai_protocol;  // use 0 for "any"
    size_t           ai_addrlen;   // size of ai_addr in bytes
    struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
    char            *ai_canonname; // full canonical hostname
    struct addrinfo *ai_next;      // linked list, next node
};

struct sockaddr
{
    unsigned short    sa_family;    // address family, AF_xxx
    char              sa_data[14];  // 14 bytes of protocol address
};

struct sockaddr_in
{
	short int          sin_family;  // Address family, AF_INET
	unsigned short int sin_port;    // Port number
	struct in_addr     sin_addr;    // Internet address
	unsigned char      sin_zero[8]; // Same size as struct sockaddr
};
Note that sin_zero (which is included to pad the structure to the length of a struct sockaddr)
should be set to all zeros with the function memset().
Also, notice that sin_family corresponds to sa_family in a struct sockaddr and should be set to “AF_INET”.
Finally, the sin_port must be in Network Byte Order (by using htons()!)

// (IPv4 only--see struct in6_addr for IPv6)
// Internet address (a structure for historical reasons)
struct in_addr
{
    uint32_t s_addr; // that's a 32-bit int (4 bytes)
};

struct sockaddr_in sa; // IPv4
struct sockaddr_in6 sa6; // IPv6

inet_pton(AF_INET, "10.12.110.57", &(sa.sin_addr)); // IPv4 presentation to network
inet_pton(AF_INET6, "2001:db8:63b3:1::3490", &(sa6.sin6_addr)); // IPv6 presentation to network

char ip4[INET_ADDRSTRLEN];  // space to hold the IPv4 string
inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN); // IPv4 network to presentation

int getaddrinfo(const char *node,     // e.g. "www.example.com" or IP
                const char *service,  // e.g. "http" or port number
                const struct addrinfo *hints,
                struct addrinfo **res);

int socket(int domain, int type, int protocol); //	domain is PF_INET or PF_INET6, 
													type is SOCK_STREAM or SOCK_DGRAM,
	 												and protocol can be set to 0 to choose the proper protocol 
	 												for the given type.

*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char const *argv[])
{
	if(argc < 2)
	{
		fprintf(stderr ,"Invalid Command! Usage: showip hostname\n");
		return 1;
	}

	struct addrinfo hints, *res,*p ;
	char ipstr[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;

    int status;

    if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
	}

	printf("IP addresses for %s:\n", argv[1]);

	for(p=res; p!=NULL; p=p->ai_next)
	{
		void *addr;
		char *ipver;
		if (p->ai_family == AF_INET)
		{ // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        }
        else
        { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
		}

		// convert the IP to a string and print it:
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("\t %s: %s\n", ipver, ipstr);
	}

	freeaddrinfo(res); // free the linked list
	return 0;
}

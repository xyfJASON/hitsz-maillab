#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define MAX_SIZE 65535

// #define onmac // mac big end, x86/linux small end
#ifdef onmac
#define swap16(x) (x)
#else
#define swap16(x) ((((x)&0xFF) << 8) | (((x) >> 8) & 0xFF))
#endif

char buf[MAX_SIZE+1];

int sendmy(int sockfd, void * buf, int len, int flags, char * error_msg, int print){
    int ret = -1;
    if((ret = send(sockfd, buf, len, flags)) == -1){
        perror(error_msg);
        exit(EXIT_FAILURE);
    }
    if(print == 1)
        printf("\033[1;32m%s\033[0m", (char *)buf);
    return ret;
}

int recvmy(int sockfd, void * buf, int len, int flags, char * error_msg){
    int r_size = -1;
    if((r_size = recv(sockfd, buf, len, 0)) == -1){
        perror(error_msg);
        exit(EXIT_FAILURE);
    }
    char *tmpbuf = (char *)buf;
    tmpbuf[r_size] = '\0';
    printf("%s", tmpbuf);
    return r_size;
}

void checkmy(char *buf, char *content, char *error_msg){
    if(strstr(buf, content) == NULL){
        perror(error_msg);
        exit(EXIT_FAILURE);
    }
}

void recv_mail()
{
    const char* host_name = "pop.qq.com"; // TODO: Specify the mail server domain name
    const unsigned short port = 110; // POP3 server port
    const char* user = "xyfjason@qq.com"; // TODO: Specify the user
    const char* pass = ""; // TODO: Specify the password
    char dest_ip[16];
    int s_fd; // socket file descriptor
    struct hostent *host;
    struct in_addr **addr_list;
    int i = 0;
    int r_size;

    // Get IP from domain name
    if ((host = gethostbyname(host_name)) == NULL)
    {
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    addr_list = (struct in_addr **) host->h_addr_list;
    while (addr_list[i] != NULL)
        ++i;
    strcpy(dest_ip, inet_ntoa(*addr_list[i-1]));

    // TODO: Create a socket,return the file descriptor to s_fd, and establish a TCP connection to the POP3 server
    if((s_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in *servaddr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
    servaddr->sin_family = AF_INET;
    servaddr->sin_port = swap16(port);
    servaddr->sin_addr = (struct in_addr){inet_addr(dest_ip)};
    bzero(servaddr->sin_zero, 8);
    connect(s_fd, (struct sockaddr *)servaddr, sizeof(struct sockaddr_in));

    // Print welcome message
    recvmy(s_fd, buf, MAX_SIZE, 0, "recv");

    // TODO: Send user and password and print server response
    sprintf(buf, "USER %s\r\n", user);
    sendmy(s_fd, buf, strlen(buf), 0, "send USER", 1);
    recvmy(s_fd, buf, MAX_SIZE, 0, "recv USER");
    checkmy(buf, "+OK", "check USER");

    sprintf(buf, "PASS %s\r\n", pass);
    sendmy(s_fd, buf, strlen(buf), 0, "send PASS", 1);
    recvmy(s_fd, buf, MAX_SIZE, 0, "recv PASS");
    checkmy(buf, "+OK", "check PASS");

    // TODO: Send STAT command and print server response
    sendmy(s_fd, "STAT\r\n", 6, 0, "send STAT", 1);
    recvmy(s_fd, buf, MAX_SIZE, 0, "recv STAT");
    checkmy(buf, "+OK", "check STAT");
    int mail_num = -1, mail_tot_size = -1;
    sscanf(buf, "+OK %d %d", &mail_num, &mail_tot_size);

    // TODO: Send LIST command and print server response
    sendmy(s_fd, "LIST\r\n", 6, 0, "send LIST", 1);
    recvmy(s_fd, buf, MAX_SIZE, 0, "recv LIST");
    checkmy(buf, "+OK", "check LIST");

    // TODO: Retrieve the first mail and print its content
    sendmy(s_fd, "RETR 1\r\n", 8, 0, "send RETR", 1);
    recvmy(s_fd, buf, MAX_SIZE, 0, "recv RETR");
    checkmy(buf, "+OK", "check RETR");

    // TODO: Send QUIT command and print server response
    sendmy(s_fd, "QUIT\r\n", 6, 0, "send QUIT", 1);
    recvmy(s_fd, buf, MAX_SIZE, 0, "recv QUIT");
    checkmy(buf, "+OK", "check QUIT");

    close(s_fd);
}

int main(int argc, char* argv[])
{
    recv_mail();
    exit(0);
}

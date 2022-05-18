#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <getopt.h>
#include "base64_utils.h"

#define MAX_SIZE 4095

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

char *file2str(const char *path){
    FILE *fp = fopen(path, "r");
    fseek(fp, 0, SEEK_END);
    int fplen = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *content = (char *)malloc(fplen);
    fread(content, 1, fplen, fp);
    fclose(fp);
    return content;
}

// receiver: mail address of the recipient
// subject: mail subject
// msg: content of mail body or path to the file containing mail body
// att_path: path to the attachment
void send_mail(const char* receiver, const char* subject, const char* msg, const char* att_path)
{
    const char* end_msg = "\r\n.\r\n";
    const char* host_name = "smtp.qq.com"; // TODO: Specify the mail server domain name
    const unsigned short port = 25; // SMTP server port
    const char* user = "***@qq.com"; // TODO: Specify the user
    const char* pass = "***"; // TODO: Specify the password
    const char* from = "***@qq.com"; // TODO: Specify the mail address of the sender
    char dest_ip[16]; // Mail server IP address
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
    
    // TODO: Create a socket, return the file descriptor to s_fd, and establish a TCP connection to the mail server
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
    recvmy(s_fd, (void *)buf, MAX_SIZE, 0, "recv");
    
    // Send EHLO command and print server response
    // TODO: Enter EHLO command here
    // TODO: Print server response to EHLO command
    const char* EHLO = "EHLO xyf\r\n";
    sendmy(s_fd, (void *)EHLO, strlen(EHLO), 0, "send EHLO", 1);
    recvmy(s_fd, (void *)buf, MAX_SIZE, 0, "recv EHLO");

    // TODO: Authentication. Server response should be printed out.
    const char* AUTH = "AUTH login\r\n";
    sendmy(s_fd, (void *)AUTH, strlen(AUTH), 0, "send AUTH", 1);
    recvmy(s_fd, (void *)buf, MAX_SIZE, 0, "recv username");
    checkmy(buf, "334 VXNlcm5hbWU6", "check username");

    char *user64 = encode_str(user); strcat(user64, "\r\n");
    sendmy(s_fd, (void *)user64, strlen(user64), 0, "send username", 1);
    recvmy(s_fd, (void *)buf, MAX_SIZE, 0, "recv password");
    checkmy(buf, "334 UGFzc3dvcmQ6", "check password");
    free(user64);

    char *pass64 = encode_str(pass); strcat(pass64, "\r\n");
    sendmy(s_fd, (void *)pass64, strlen(pass64), 0, "send password", 1);
    recvmy(s_fd, (void *)buf, MAX_SIZE, 0, "recv AUTH");
    checkmy(buf, "Authentication successful", "check AUTH");
    free(pass64);

    // TODO: Send MAIL FROM command and print server response
    sprintf(buf, "MAIL FROM:<%s>\r\n", from);
    sendmy(s_fd, (void *)buf, strlen(buf), 0, "send MAIL FROM", 1);
    recvmy(s_fd, (void *)buf, MAX_SIZE, 0, "recv MAIL FROM");
    checkmy(buf, "250 OK", "check password");

    // TODO: Send RCPT TO command and print server response
    sprintf(buf, "RCPT TO:<%s>\r\n", receiver);
    sendmy(s_fd, (void *)buf, strlen(buf), 0, "send RCPT TO", 1);
    recvmy(s_fd, (void *)buf, MAX_SIZE, 0, "recv RCPT TO");
    checkmy(buf, "250 OK", "check password");
    
    // TODO: Send DATA command and print server response
    sendmy(s_fd, "DATA\r\n", 6, 0, "send DATA", 1);
    recvmy(s_fd, (void *)buf, MAX_SIZE, 0, "recv DATA");
    checkmy(buf, "354", "check DATA");

    // TODO: Send message data
    sprintf(buf, "From: %s\r\nTo: %s\r\nContent-Type: multipart/mixed; boundary=qwertyuiopasdfghjklzxcvbnm\r\n", from, receiver);
    if(subject != NULL) strcat(buf, "Subject: "), strcat(buf, subject), strcat(buf, "\r\n\r\n");
    sendmy(s_fd, (void *)buf, strlen(buf), 0, "send DATA header", 1);    
    
    if(msg != NULL){
        sprintf(buf, "--qwertyuiopasdfghjklzxcvbnm\r\nContent-Type:text/plain\r\n\r\n");
        sendmy(s_fd, (void *)buf, strlen(buf), 0, "send DATA msg", 1);
        if(access(msg, F_OK) == 0){
            char *content = file2str(msg);
            sendmy(s_fd, (void *)content, strlen(content), 0, "send DATA msg content", 0);
            free(content);
        }
        else    sendmy(s_fd, (void *)msg, strlen(msg), 0, "send DATA msg content", 1);
        sendmy(s_fd, "\r\n", 2, 0, "", 1);
    }
    
    if(att_path != NULL){
        sprintf(buf, "--qwertyuiopasdfghjklzxcvbnm\r\nContent-Type:application/octet-stream\r\nContent-Transfer-Encoding: base64\r\nContent-Disposition: attachment; name=%s\r\n\r\n", att_path);
        sendmy(s_fd, (void *)buf, strlen(buf), 0, "send DATA attach", 1);
        FILE *fp = fopen(att_path, "r");
        if(fp == NULL){
            perror("file not exist");
            exit(EXIT_FAILURE);
        }
        FILE *fp64 = fopen("tmp.attach", "w");
        encode_file(fp, fp64);
        fclose(fp); fclose(fp64);
        char *attach = file2str("tmp.attach");
        sendmy(s_fd, (void *)attach, strlen(attach), 0, "send DATA attachment files", 0);
        free(attach);
    }
    sprintf(buf, "--qwertyuiopasdfghjklzxcvbnm\r\n");
    sendmy(s_fd, (void *)buf, strlen(buf), 0, "send DATA last", 1);
    
    // TODO: Message ends with a single period
    sendmy(s_fd, (void *)end_msg, strlen(end_msg), 0, "send .", 1);
    recvmy(s_fd, (void *)buf, MAX_SIZE, 0, "recv .");
    checkmy(buf, "250 OK", "check .");

    // TODO: Send QUIT command and print server response
    sendmy(s_fd, "QUIT\r\n", 6, 0, "send QUIT", 1);
    recvmy(s_fd, (void *)buf, MAX_SIZE, 0, "recv QUIT");
    checkmy(buf, "221 Bye", "check QUIT");

    close(s_fd);
}

int main(int argc, char* argv[])
{
    int opt;
    char* s_arg = NULL;
    char* m_arg = NULL;
    char* a_arg = NULL;
    char* recipient = NULL;
    const char* optstring = ":s:m:a:";
    while ((opt = getopt(argc, argv, optstring)) != -1)
    {
        switch (opt)
        {
        case 's':
            s_arg = optarg;
            break;
        case 'm':
            m_arg = optarg;
            break;
        case 'a':
            a_arg = optarg;
            break;
        case ':':
            fprintf(stderr, "Option %c needs an argument.\n", optopt);
            exit(EXIT_FAILURE);
        case '?':
            fprintf(stderr, "Unknown option: %c.\n", optopt);
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "Unknown error.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
    {
        fprintf(stderr, "Recipient not specified.\n");
        exit(EXIT_FAILURE);
    }
    else if (optind < argc - 1)
    {
        fprintf(stderr, "Too many arguments.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        recipient = argv[optind];
        send_mail(recipient, s_arg, m_arg, a_arg);
        exit(0);
    }
}

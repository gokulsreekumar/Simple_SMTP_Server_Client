#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define BUFSIZE 80*50 + 608
#define LINE_SIZE 80
#define MAX_LINES 50
#define MAX_BODY_SIZE LINE_SIZE * MAX_LINES + 1

// Message Structure
// TYPES:

#define AUTH 0
#define EMAIL 1
#define STATUS 2

struct message
{
    int type;
    char data[BUFSIZE];
};

int verify_email(char *email)
{

    int username = 0;
    int domain = 0;

    int flag = 0;

    int i=0;
    while((i++)<strlen(email)) 
    {
        if(email[i] == '@') {
            flag = 1;
            continue;
        }

        if (flag == 0)
            username++;
        else
            domain++;

        if (domain >= 1)
            break;

    }

    return username > 0 && domain > 0;
}

void timeDelay() {
    int tick = 0;
    while((tick++)<1000) {
        int tick_cnt = tick+1;
    }
}

void userOptions(int sockfd)
{
    // 1. Send Mail
    // 2. Quit
    int option;

    char buffer[6000];
    char input_line[100];

    bzero(input_line, sizeof(input_line));
    bzero(buffer, sizeof(buffer));

    while (1)
    {
        struct message recv_message;
        bzero(recv_message.data, sizeof(recv_message.data));

        printf("1. Send Mail\n2. Quit\nEnter your option: ");
        scanf("%d", &option);

        if (option == 1)
        {

            int lineNum = 0;

            char from[50], to[50], subject[50], body[MAX_BODY_SIZE];
            char buffer[LINE_SIZE] = {0};
            
            char fieldName[50];

            scanf("%s%*c", fieldName);

            if(strcmp(fieldName, "From:")) {
                printf("Incorrect format\n");
                continue;
            }

            scanf("%s%*c", from);

            scanf("%s%*c", fieldName);

            if(strcmp(fieldName, "To:")) {
                printf("Incorrect format\n");
                continue;
            }

            scanf("%s%*c", to);
            
            scanf("%s%*c", fieldName);

            if(strcmp(fieldName, "Subject:")) {
                printf("Incorrect format\n");
                continue;
            }

            scanf("%[^\n]%*c", subject);

            if (!verify_email(to) || !verify_email(from))
            {
                printf("Incorrect format\n");
                continue;
            }
            
            while (strcmp(buffer, ".") != 0 && lineNum < MAX_BODY_SIZE) {
                scanf("%[^\n]%*c", buffer);
                lineNum += sprintf(body + lineNum, "%s\n", buffer);
            }

            char data[BUFSIZE];

            // Formatting the Email
            lineNum = 0;
            lineNum += sprintf(data + lineNum, "From: %s\n", from);
            lineNum += sprintf(data + lineNum, "To: %s\n", to);
            lineNum += sprintf(data + lineNum, "Subject: %s\n", subject);
            lineNum += sprintf(data + lineNum, "%s\n", body);
            lineNum += sprintf(data + lineNum, ".\n");

            // Sending the Email
            struct message send_message;
            bzero(send_message.data, sizeof(send_message.data));

            strcpy(send_message.data, data);
            send_message.type = EMAIL;

            send(sockfd, &send_message, sizeof(send_message), 0);

            timeDelay();


            recv(sockfd, &recv_message, sizeof(recv_message), 0);
            printf("%s\n", recv_message.data);


        }
        else if (option == 2)
        {
            break;
        }
        else
        {
            printf("Invalid option!\nPlease try again\n");
        }
    }
}

void connectToServer(int *sockfd, struct sockaddr_in *server_address, int my_port)
{
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (*sockfd == -1)
    {
        perror("Socket creation failed");
        exit(1);
    }

    server_address->sin_family = AF_INET;
    server_address->sin_port = htons(my_port);
    server_address->sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(*sockfd, (struct sockaddr *)server_address, sizeof(struct sockaddr)) == -1)
    {
        perror("Failed to establish connection\n");
        exit(1);
    }

    printf("Connected to Server\n");
}

int main(int argc, char* argv[])
{
    int my_port;
    if(argc<2) {

        printf("No Port provided!\n");
        exit(1);
    
    } else if(argc==2) {
        
        if (sscanf(argv[1], "%d", &my_port) != 1)
        {
            printf("Invalid PORT number\n");
            exit(1);
        }

    }

    int sockfd;
    struct sockaddr_in server_address;

    connectToServer(&sockfd, &server_address, my_port);
    
    while(1) {

        char username[100];
        char password[100];

        bzero(username, sizeof(username));
        bzero(password, sizeof(password));    

        printf("Please enter your username:");
        scanf("%s", username);
        getchar();

        printf("Please enter the password:");
        scanf("%s", password);
        getchar();


        // CHECK CREDS: SEND AUTH TYPE MESSAGES
        
        struct message send_message;
        bzero(send_message.data, sizeof(send_message.data));

        send_message.type = AUTH;
        sprintf(send_message.data, "%s %s", username, password);

        send(sockfd, &send_message, sizeof(send_message), 0);

        timeDelay();

        struct message recv_message;
        bzero(recv_message.data, sizeof(recv_message.data));

        recv(sockfd, &recv_message, sizeof(recv_message), 0);

        if(strcmp("Success", recv_message.data)==0) {

            printf("You are succesfully logged in.\n");
            break;

        } else {

            printf("%s", recv_message.data);
            connectToServer(&sockfd, &server_address, my_port);

        }

    }

    // Menu Driven Function
    userOptions(sockfd);

    return 0;
}

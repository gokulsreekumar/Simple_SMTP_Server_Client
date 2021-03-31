#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#define PORT 8080
#define BUFSIZE 80*50 + 608

#define MAX_NUMBER_CLIENTS 1024

struct userInfo {

    char username[100];
    char password[100];

} userDirectory[MAX_NUMBER_CLIENTS];

int totalNumberOfUsers;

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

void startServer(int *sockfd, struct sockaddr_in *server_addr, int);
void acceptConnection(fd_set *master, int *fdmax, int sockfd, struct sockaddr_in *client_addr);

void timeDelay() {
    int tick = 0;
    while((tick++)<1000) {
        int tick_cnt = tick+1;
    }
}


void recvMail(int i, fd_set *fds, int sockfd, int max_fd) {
	
    // Time
    int hours, minutes, day, month, year;

    // time_t is arithmetic time type
    time_t now;

    struct message recv_message;
    bzero(recv_message.data, sizeof(recv_message.data));

    int n = recv(i, &recv_message, sizeof(recv_message), 0);

    // Obtain current time
    // time() returns the current time of the system as a time_t value
    time(&now);
 
    // Convert to local time format and print to stdout
    ctime(&now);
 
    // localtime converts a time_t value to calendar time
    struct tm *local = localtime(&now);
 
    hours = local->tm_hour;          // get hours since midnight (0-23)
    minutes = local->tm_min;         // get minutes passed after the hour (0-59)
 
    day = local->tm_mday;            // get day of month (1 to 31)
    month = local->tm_mon + 1;       // get month of year (0 to 11)
    year = local->tm_year + 1900;    // get year since 1900

    if(n <= 0) {
        close(i);
        FD_CLR(i, fds);
        return;
    }

    char from[300], to[300], prefix[50], suffix[50];

	sscanf(recv_message.data, "%[^\n] %[^\n]", from, to);

	sscanf(to, "%s %s", prefix, suffix);
	char *receiver = strtok(suffix, "@");
	printf("To username: %s\n", receiver);


    struct message send_message;
    bzero(send_message.data, sizeof(send_message.data));  

    int j=0; int flag = 0;

    while(j<totalNumberOfUsers) {
        
        if(strcmp(receiver, userDirectory[j].username)==0) {

            strcpy(send_message.data, "Mail sent successfully");
            send(i, &send_message, sizeof(send_message), 0);
            flag = 1;
            break;

        }

        j++;

    }

    if(flag==0) {
        strcpy(send_message.data, "Failed to Send the Email.");
        send(i, &send_message, sizeof(send_message), 0);
        return;
    }
    

	char filename[200];
	bzero(filename, sizeof(filename));

	sprintf(filename, "%s/mymailbox.mail", receiver);
    

	FILE *fp = fopen(filename, "a");

    int lineNum = 1;
	char * line = strtok(recv_message.data, "\n");
	fprintf(fp, "%s\n", line);
	
	while( line != NULL ) {

		lineNum++;
		line = strtok(NULL, "\n");
		if(line == NULL || strcmp(line, ".")==0)
			break;
		
		fprintf(fp, "%s\n", line);
        bzero(line, sizeof(line));

		if(lineNum == 3) {

			char time_line[300];
			bzero(time_line, sizeof(time_line));
			sprintf(time_line, "Received: %d/%d/%d:%d:%d\n", day, month, year, hours, minutes);
			fprintf(fp, "%s", time_line);
		
        }

	}

    fprintf(fp, ".\n");

    fclose(fp);
        	
}

int main(int argc, char* argv[])
{
    int my_port;
    if(argc<2) {

        printf("No Port provided as argument\n");
        exit(1);
    
    } else if(argc==2) {
        
        if (sscanf(argv[1], "%d", &my_port) != 1)
        {
            printf("Invalid PORT number\n");
            exit(1);
        }

    }

    fd_set master, read_fds;

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    int sockfd = 0;
    struct sockaddr_in server_addr, client_addr;

    startServer(&sockfd, &server_addr, my_port);

    FD_SET(sockfd, &master);
    int fdmax;
    fdmax = sockfd;

    char fusername[100], fpassword[100];
    bzero(fusername, sizeof(fusername));
    bzero(fpassword, sizeof(fpassword));

    int i = 0;

    // Fill userDirectory 
    FILE *fileCredentials = fopen("logincred.txt", "r");

    while (fscanf(fileCredentials, "%[^,]%*c %[^\n]%*c", fusername, fpassword) != EOF)
    {
        strcpy(userDirectory[i].username, fusername);
        strcpy(userDirectory[i].password, fpassword);
        bzero(fusername, sizeof(fusername));
        bzero(fpassword, sizeof(fpassword));
        i++;
    }

    fclose(fileCredentials);

    totalNumberOfUsers = i;

    while (1)
    {
        read_fds = master;
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("select");
            exit(4);
        }
        for (i = 0; i <= fdmax; i++)
        {
            if (FD_ISSET(i, &read_fds))
            {
                if (i == sockfd)
                    acceptConnection(&master, &fdmax, sockfd, &client_addr);
                else
                    recvMail(i, &master, sockfd, fdmax);
            }
        }
    }

    close(sockfd);
    return 0;
}

void startServer(int *sockfd, struct sockaddr_in *server_addr, int my_port)
{
    if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket");
        exit(1);
    }

    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(my_port);
    server_addr->sin_addr.s_addr = INADDR_ANY;

    int flag = 1;
	if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) == -1) {
		perror("error in setsockopt");
		exit(1);
	}

    if (bind(*sockfd, (struct sockaddr *)server_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("Unable to bind");
        exit(1);
    }

    if (listen(*sockfd, 10) == -1)
    {
        perror("listen");
        exit(1);
    }

    printf("SMTP Server Running at Port: %d ...\n", my_port);
    fflush(stdout);
}

void acceptConnection(fd_set *master, int *fdmax, int sockfd, struct sockaddr_in *client_addr)
{

    socklen_t addrlen = sizeof(struct sockaddr_in);
    int newsockfd;

    if ((newsockfd = accept(sockfd, (struct sockaddr *)client_addr, &addrlen)) == -1)
    {
        perror("accept");
        exit(1);
    }
    else
    {
        FD_SET(newsockfd, master);
        if (newsockfd > *fdmax)
            *fdmax = newsockfd;
        printf("New connection at %s : %d \n", inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));
    }

    // CHECK VALIDITY OF CREDS
    struct message send_message, recv_message;
    bzero(recv_message.data, sizeof(recv_message.data));
    bzero(send_message.data, sizeof(send_message.data));

    recv(newsockfd, &recv_message, sizeof(recv_message), 0);

    char username[100];
    char password[100];

    bzero(username, sizeof(username));
    bzero(password, sizeof(password));

    sscanf(recv_message.data, "%s %s", username, password);
    printf("username:%s password:%s\n", username, password);

    int i = 0;
    int flag = 0;

    while(i < totalNumberOfUsers) {

        printf("uname: %s, psswd: %s\n", userDirectory[i].username, userDirectory[i].password);
    
        if(strcmp(username, userDirectory[i].username)==0) {

            if(strcmp(password, userDirectory[i].password)==0) {
                
                send_message.type = AUTH;
                sprintf(send_message.data, "Success");

                send(newsockfd, &send_message, sizeof(send_message), 0);

                flag = 1;

                break;

            } else {

                send_message.type = AUTH;
                sprintf(send_message.data, "Incorrect Password");

                send(newsockfd, &send_message, sizeof(send_message), 0);

                break;

            }

        }

        i++;

    }

    if(flag!=1) {

        send_message.type = AUTH;
        sprintf(send_message.data, "Incorrect Username");

        send(newsockfd, &send_message, sizeof(send_message), 0);

        FD_CLR(newsockfd, master);
        close(newsockfd);

    }


}


void broadcast(int i, int sockfd, int size, struct message recv_message, fd_set *master, int fdmax)
{
    for(int j = 0; j <= fdmax; j++)
        if (FD_ISSET(j, master)){
            if (j != sockfd && j != i) {
                if (send(j, &recv_message, size, 0) == -1) {
                    perror("Sending failed");
                }
            }
        }
}


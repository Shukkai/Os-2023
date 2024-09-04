#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>


#define BUFFER_SIZE 1024
#define PORT 80
using namespace std;
void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int createSocket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        error("socket()");
    }
    return sockfd;
}

void connectToServer(int sockfd, const char *hostname) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname, "80", &hints, &res) != 0) {
        error("getaddrinfo()");
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        error("connect()");
    }

    freeaddrinfo(res);
    printf("======== Socket ==========\n");
    printf("Connected to server\n");
}

void sendRequest(int sockfd, const char *hostname, const char *path) {
    char message[BUFFER_SIZE];
    snprintf(message, BUFFER_SIZE, "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, hostname);

    if (send(sockfd, message, strlen(message), 0) == -1) {
        perror("send()");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("======== Sending HTTP request ==========\n%s", message);
}


void receiveResponse(int sockfd, char *buffer) {
    ssize_t bytesReceived;
    size_t totalReceived = 0;

    while ((bytesReceived = recv(sockfd, buffer + totalReceived, BUFFER_SIZE - totalReceived - 1, 0)) > 0) {
        totalReceived += bytesReceived;
        buffer[totalReceived] = '\0';

        // Check if the response is complete (end of headers and start of body)
        char *bodyStart = strstr(buffer, "\r\n\r\n");
        if (bodyStart != nullptr) {
            printf("======== Receiving the response ==========\n%s", buffer);
        }
        else{
            break;
        }
    }

    if (bytesReceived == -1) {
        perror("recv()");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}



void parseAndPrintHyperlinks(const char *html) {
    const char *start = html;
    int hyperlinkCount = 0;

    printf("======= Hyperlinks ========\n");

    while ((start = strstr(start, "<a")) != NULL) {
        const char *hrefStart = strstr(start, "href=\"");
        if (hrefStart != NULL) {
            hrefStart += 6; // Move to the beginning of the actual URL
            const char *hrefEnd = strchr(hrefStart, '\"');
            if (hrefEnd != NULL) {
                size_t hrefLength = hrefEnd - hrefStart;
                char href[BUFFER_SIZE];
                strncpy(href, hrefStart, hrefLength);
                href[hrefLength] = '\0';

                printf("Found href: %s\n", href);
                hyperlinkCount++;
            }
        }

        // Move to the next potential start tag
        start++;
    }

    printf("Total hyperlinks: %d\n", hyperlinkCount);
    printf("===========================\n");
}


int main() {
    // Input URL
    printf("Please enter the URL:\n");
    char inputUrl[BUFFER_SIZE];
    fgets(inputUrl, BUFFER_SIZE, stdin);

    // Parse input URL into hostname and path
    char hostname[BUFFER_SIZE];
    char path[BUFFER_SIZE];
    sscanf(inputUrl, "%[^/]/%[^\n]", hostname, path);

    int sockfd = createSocket();
    connectToServer(sockfd, hostname);

    // Send HTTP GET request
    sendRequest(sockfd, hostname, path);

    char buffer[BUFFER_SIZE] = {'\0'};
    receiveResponse(sockfd, buffer);

    // Debug print: Print received HTML content
    std::cout << "Received HTML:\n" << buffer << std::endl;

    // Parse and display hyperlinks
    parseAndPrintHyperlinks(buffer);

    close(sockfd);

    return 0;
}
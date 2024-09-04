#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define PORT 80

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
    snprintf(message, BUFFER_SIZE, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, hostname);

    if (send(sockfd, message, strlen(message), 0) == -1) {
        error("send()");
    }
    printf("Sending HTTP request\n");
}

void receiveResponse(int sockfd, char *buffer) {
    ssize_t bytesReceived = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
    if (bytesReceived == -1) {
        error("recv()");
    }

    buffer[bytesReceived] = '\0';

    printf("Receiving the response\n");
}

void parseHyperlinks(const char *html) {
    const char *start = html;
    int hyperlinkCount = 0;

    printf("=======Hyperlinks ========\n");

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

                // Check if the href is not empty and doesn't start with "http://" or "https://"
                if (strlen(href) > 0 && strncmp(href, "http://", 7) != 0 && strncmp(href, "https://" != 0) {
                    printf("%s\n", href);
                    hyperlinkCount++;
                }
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

    // Check if the user entered "http://"
    if (strncmp(inputUrl, "http://", 7) != 0) {
        fprintf(stderr, "Error: Please enter the URL in the format http://hostname/path\n");
        return EXIT_FAILURE;
    }

    // Parse input URL into hostname and path
    char hostname[BUFFER_SIZE];
    char path[BUFFER_SIZE];
    sscanf(inputUrl + 7, "%[^/]/%[^\n]", hostname, path);

    int sockfd = createSocket();
    connectToServer(sockfd, hostname);

    // Send HTTP GET request
    sendRequest(sockfd, hostname, path);

    char buffer[BUFFER_SIZE] = {'\0'};
    receiveResponse(sockfd, buffer);

    // Parse and display hyperlinks
    parseHyperlinks(buffer);

    close(sockfd);

    return 0;
}
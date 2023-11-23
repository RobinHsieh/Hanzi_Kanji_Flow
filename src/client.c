#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024



int server_fd = 0;


void *receiveMessages(void *arg) {

    char buffer[BUFFER_SIZE];
    int valread;

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        valread = recv(server_fd, buffer, BUFFER_SIZE - 1, 0);
        if (valread > 0) {
            buffer[valread] = '\0';
            printf("Server: %s\n", buffer);
        }
        else {
            printf("Server connection closed.\n");
            break;
        }
    }
    return NULL;
}


int main() {

    struct sockaddr_in serv_addr;
    char message[BUFFER_SIZE];
    pthread_t recv_thread;

    // 創建 socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // 將 IPv4 地址從文本轉換為二進制形式
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // 連接到服務器
    if (connect(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // 創建接收消息的線程
    if (pthread_create(&recv_thread, NULL, receiveMessages, NULL) != 0) {
        printf("Failed to create thread\n");
        return -1;
    }

    // 處理用戶輸入的消息
    while (1) {
        fgets(message, BUFFER_SIZE, stdin);
        message[strcspn(message, "\n")] = 0;  // 去除換行符
        send(server_fd, message, strlen(message), 0);
        printf("Message sent\n");
    }

    close(server_fd);
    return 0;
}

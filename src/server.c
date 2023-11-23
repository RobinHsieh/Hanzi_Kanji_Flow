#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int opt = 1;
    int master_socket_fd;  // 主 socket，用於接受新的連接請求
    int addrlen;           // 用於存儲地址結構的大小
    int new_socket;        // 新連接的 socket 描述符
    int client_socket_fd_array[MAX_CLIENTS]; // 存儲所有客戶端的 socket 描述符
    int max_clients = MAX_CLIENTS;  // 允許的最大客戶端數量
    int activity;          // 存儲 select() 函數的返回值，表示 sockets 上的活動
    int valread;           // 存儲從 socket 讀取的字節數
    int socket_fd;         // 臨時變量，用於引用當前正在處理的客戶端 socket 描述符
    int max_socket_fd;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];

    fd_set read_fd_set;

    // 初始化所有客戶端 socket
    for (int i = 0; i < max_clients; i++) {
        client_socket_fd_array[i] = 0;
    }

    /* 創建 master socket
     * return: socket file descriptor
     * AF_INET: IPv4 Internet protocols
     * SOCK_STREAM: Provides sequenced, reliable, bidirectional, connection-mode byte streams (TCP).
     */
    
    if ((master_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    /* 設置 socket 選項 */
    if (setsockopt(master_socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // 設置 socket 地址
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 綁定 socket 到 localhost 的 8080 端口
    if (bind(master_socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

    // 嘗試指定最大的 10 個待接受連接
    if (listen(master_socket_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // 接受接入的連接
    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    while (1) {
        // 更新前，先清空 socket 集合 (read_fd_set –– read file descriptors set)
        FD_ZERO(&read_fd_set);

        // 添加 master socket 到集合 (read_fd_set –– read file descriptors set)
        FD_SET(master_socket_fd, &read_fd_set);
        max_socket_fd = master_socket_fd;

        // 重新添加更新後的子 sockets 到集合
        for (int i = 0; i < max_clients; i++) {
            // socket 描述符
            socket_fd = client_socket_fd_array[i];

            // 如果有效則添加到讀取列表
            if (socket_fd > 0)
                FD_SET(socket_fd, &read_fd_set);

            // 最大的文件描述符數
            if (socket_fd > max_socket_fd)
                max_socket_fd = socket_fd;
        }

        // 等待活動在 sockets 上，無超時限制
        activity = select(max_socket_fd + 1, &read_fd_set, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            printf("select error");
        }

        // 如果有活動在 master socket，是一個入站連接
        if (FD_ISSET(master_socket_fd, &read_fd_set)) {
            if ((new_socket = accept(master_socket_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // 通知用戶 socket 號 - 用於識別
            printf("New connection, socket fd is %d, ip is: %s, port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // 添加新的 socket 到數組
            for (int i = 0; i < max_clients; i++) {
                if (client_socket_fd_array[i] == 0) {
                    client_socket_fd_array[i] = new_socket;
                    printf("Adding to list of sockets as %d\n", i);
                    break;
                }
            }
        }
        // 否則進行 I/O 操作
        else {
            for (int i = 0; i < max_clients; i++) {
                socket_fd = client_socket_fd_array[i];

                if (FD_ISSET(socket_fd, &read_fd_set)) {
                    
                    memset(buffer, 0, BUFFER_SIZE);
                    // 檢查是否是關閉了連接，是則關閉
                    if ((valread = recv(socket_fd, buffer, BUFFER_SIZE - 1, 0)) == 0) {
                        getpeername(socket_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
                        printf("Host disconnected, ip %s, port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                        close(socket_fd);
                        client_socket_fd_array[i] = 0;
                    }

                    // 否則回顯該消息
                    else {
                        buffer[valread] = '\0';
                        for (int j = 0; j < max_clients; j++) {
                            send(client_socket_fd_array[j], buffer, strlen(buffer), 0);
                        }
                    }
                }
            }
        }
    }

    return 0;
}

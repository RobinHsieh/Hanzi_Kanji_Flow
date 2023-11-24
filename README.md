# Socket Programming

109601005 大氣四 謝文喨

## 程式功能
- 多人聊天室
- 瀏覽器GUI介面

## 程式架構
### Server 端
`server.c`  
#### 執行重點流程
1. create welcome socket
```c
master_socket_fd = socket(AF_INET, SOCK_STREAM, 0)
```
2. bind welcome socket to IP and port
```c
bind(master_socket_fd, (struct sockaddr *)&address, sizeof(address))
```

3. listen 連線請求、設定最大連線數
```c
listen(master_socket_fd, 10)
```

_**無限迴圈開始，迴圈內部執行以下流程：**_

4. 更新連線 sockets (連線 sockets 的 file descriptor)
```c
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
```

5. 偵測連線 sockets 的 file descriptor 有無活動
```c
activity = select(max_socket_fd + 1, &read_fd_set, NULL, NULL, NULL);
```

6. 如果 welcome socket 有活動，表示有新的連線請求
```c
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
```

7. 如果是其他 socket 有活動，表示有新的訊息，接收任一 socket 的訊息，並廣播給其他 sockets
```c
valread = recv(socket_fd, buffer, BUFFER_SIZE - 1, 0)
buffer[valread] = '\0';
for (int j = 0; j < max_clients; j++) {
    send(client_socket_fd_array[j], buffer, strlen(buffer), 0);
}
```


### Client 後端
`GUI.js`
#### 執行重點流程
1. 創建 HTTP 服務器，並在其上初始化 WebSocket 服務器
```js
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });
```

2. 創建並連接到 TCP 伺服器
```js
const tcpClient = new net.Socket();
tcpClient.connect(PORT, '172.20.10.2', () => {
    console.log('Connected to TCP server');
});
```

3. 處理 WebSocket 連接、訊息收發
```js
wss.on('connection', (ws) => {
    ws.on('message', (message) => {
        tcpClient.write(message);
    });

    tcpClient.on('data', (data) => {
        ws.send(data.toString());
    });
});
```

4. 啟動 HTTP 服務器
```js
server.listen(3000, () => {
    console.log('WebSocket server started on port 3000');
});
```


### Client 前端
`GUI.html`
#### 執行重點流程

_**下方為`<script>`標籤內的程式碼**_

1. 選擇 DOM 元素
```js
const messages = document.getElementById('messages');
const messageInput = document.getElementById('messageInput');
```

2. 建立 WebSocket 連線 (172.20.10.2 為 server IP)
```js
const socket = new WebSocket('ws://172.20.10.2:3000');
```

3. 處理收到的消息：事件 `onmessage` 被觸發時，將收到的訊息顯示在瀏覽器上
```js
socket.onmessage = function(event) {
    messages.innerHTML += `<div>${event.data}</div>`;
};
```

4. 處理發送消息
```js
function sendMessage() {
    const message = messageInput.value;
    socket.send(message);
    messageInput.value = '';
}
```
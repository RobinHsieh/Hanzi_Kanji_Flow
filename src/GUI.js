const express = require('express');
const http = require('http');
const WebSocket = require('ws');
const net = require('net');

const PORT = 8080;

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

const tcpClient = new net.Socket();
tcpClient.connect(PORT, '172.20.10.2', () => {
    console.log('Connected to TCP server');
});

/* 
 * wss 是 WebSocket.Server 的實例，用於建立連線
 * ws 是 WebSocket 的實例，用於傳送訊息
 * 當名為 connection 的事件被觸發時，callback function `(ws) => {...}` 會被執行
 * wss.on() 這個函數會提供 `ws` 這個物件給 callback function `(ws) => {...}` 做為參數
 */

/* 
 * JavaScript 實現非阻塞異步操作主要通過其事件循環（Event Loop）和回調函數
 * JavaScript 代碼在主線程上運行
 * 除了主線程外，還有其他現成在處裡事件循環
 * 當一個異步事件被觸發、並完成時，與之相關的回調函數被添加到任務隊列中，
 * 　　事件循環會監控這個隊列，並在適當的時候將回調函數從隊列中取出，放回主線程執行
 */
wss.on('connection', (ws) => {
    ws.on('message', (message) => {
        tcpClient.write(message);
    });

    tcpClient.on('data', (data) => {
        ws.send(data.toString());
    });
});

server.listen(3000, () => {
    console.log('WebSocket server started on port 3000');
});

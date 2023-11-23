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

/* 當名為 connection 的事件被觸發時，callback function `(ws) => {...}` 會被執行
 * `ws` 是一個 WebSocket 物件，代表一個 WebSocket 連線
 * wss.on() 這個函數會提供 `ws` 這個物件給 callback function `(ws) => {...}` 做為參數
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

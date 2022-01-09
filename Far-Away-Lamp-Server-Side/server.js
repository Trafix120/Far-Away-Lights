'use strict';

const express = require('express');
const { Server } = require('ws');

const PORT = process.env.PORT || 3000;
const INDEX = '/index.html';
const DEFAULT_TEXT_MESSAGE = "SSSite is Online!";

// Server Boilerplate code
const server = express()
  .use((req, res) => res.sendFile(INDEX, { root: __dirname }))
  .listen(PORT, () => console.log(`Listening on ${PORT}`));

const wss = new Server({ server });

// On a client's connection
wss.on('connection', (ws) => {
  const clientId = Math.random();
  ws.clientId = clientId;
  ws.isAlive = true;

  console.log(`>> CLIENT CONNECTED - ID: ${clientId}`);
  
  // Broadcasting to Website
  wss.clients.forEach((client) => {
    if (client.clientId == clientId) {
      const message = {
        isText: true,
        text: DEFAULT_TEXT_MESSAGE
      }
      client.send(JSON.stringify(message), { binary: false });
    }
  });

  // Broadcasting to message
  ws.on('message', (dataAsBuffer, isBinary) => {
    console.log(`>> DATA RECEIVED - ID: ${clientId}`);
    const data = JSON.parse(dataAsBuffer.toString());

    ws.groupId = data.groupId;
    ws.isAlive = true;

    wss.clients.forEach((client) => {
      if (client.groupId == ws.groupId) {
        const message = {
          isText: false,
          lampState: data.lampState,
        }
        client.send(JSON.stringify(message), { binary: false });
      }
    });

  });

  ws.on("pong", () => {
    ws.isAlive = true;
  });

  ws.on('close', () => {
    console.log(`>> CLIENT DISCONNECTED - ID: ${clientId}`)
    ws.isAlive = false;
  });
});

setInterval(() => {
  console.log(">> PINGING EVERYONE");
  wss.clients.forEach((ws) => {    
    if (ws.isAlive == false) {
      console.log(`Err: Heartbeat not received - ID: ${ws.clientId}`);
      return ws.terminate();
    }
    ws.isAlive = false;
    ws.ping();
  });
}, 6000);

setInterval(() => {
  console.log(">> SENDING MESSAGE TO PREVENT HEROKU TIMEOUT");
  wss.clients.forEach((ws) => {
    if (ws.isAlive == true) {
      const message = {
        isText: true,
        text: DEFAULT_TEXT_MESSAGE
      }
      ws.send(JSON.stringify(message), { binary: false });
    }
  });
}, 1740000) // 29 minutes in miliseconds






/*
// Message Format (Lamp to Server)
{
  "groupId": 101,
  "lampState": false
}

// Message Format (Server to Lamp)
{
  "isText" : true,
  "text": "Connection is established"
}
or
{
  "isText": false,
  "lampState": false
}



// Resources
Websocket Node Js Library: https://github.com/websockets/ws
*/
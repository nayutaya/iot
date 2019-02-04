#!/usr/bin/env node

const express = require("express");

const MQTT_SERVER_URL = process.env.MQTT_SERVER_URL;
const STATE_TOPIC     = "sensor/restroom/state";
console.log("MQTT_SERVER_URL:", MQTT_SERVER_URL);
console.log("STATE_TOPIC:", STATE_TOPIC);

const {mqttClient, stateSubject} = require("./mqtt")({
  mqttServerUrl: MQTT_SERVER_URL,
  stateTopic: STATE_TOPIC,
});

const {stateHistorySubject} = require("./controller")({stateSubject})

stateSubject.subscribe((state) => {
  console.log("[Rx] state:", state);
});

stateHistorySubject.subscribe((stateHistory) => {
  // console.log("[Rx] stateHistory:", stateHistory);
});

const app = express();
const expressWs = require("express-ws")(app);

app.get("/state.json", (req, res, next) => {
  res.json({
    Time: new Date().getTime(),
    State: stateSubject.value,
  });
});

app.get("/state/history.json", (req, res, next) => {
  res.json({
    Time: new Date().getTime(),
    StateHistory: stateHistorySubject.value,
  });
});

app.ws("/", (ws, req) => {
  ws.on("message", (msg) => {
    // nop
  });

  ws.send(JSON.stringify(stateHistorySubject.value));
});

stateSubject.subscribe((state) => {
  expressWs.getWss().clients.forEach((client) => {
    client.send(JSON.stringify([state]));
  });
});

const WEB_API_HOST = "0.0.0.0";
const WEB_API_PORT = 8080;
const server = app.listen(WEB_API_PORT, WEB_API_HOST, () => {
  console.log("[Web] http://" + server.address().address + ":" + server.address().port + "/");
});

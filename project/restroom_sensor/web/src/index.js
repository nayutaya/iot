#!/usr/bin/env node

const express = require("express");
const mqtt    = require("mqtt");
const Rx      = require("rxjs");

const MQTT_SERVER_URL = process.env.MQTT_SERVER_URL;
const STATE_TOPIC     = "sensor/restroom/state";
console.log("MQTT_SERVER_URL:", MQTT_SERVER_URL);
console.log("STATE_TOPIC:", STATE_TOPIC);

const client = mqtt.connect(MQTT_SERVER_URL);

client.on("connect", () => {
  console.log("[MQTT] connect");

  client.subscribe(STATE_TOPIC, (err) => {
    console.log("[MQTT] subscribe:", STATE_TOPIC);
  });
});

const stateSubject = new Rx.BehaviorSubject(null);
client.on("message", (topic, message) => {
  if ( topic === STATE_TOPIC ) {
    stateSubject.next(JSON.parse(message));
  }
});
stateSubject.subscribe((state) => {
  console.log("[Rx] state:", state);
});

const app = express();

app.get("/state.json", (req, res, next) => {
  res.json({
    Time: new Date().getTime(),
    State: stateSubject.value,
  });
});

const server = app.listen(8080, () => {
  console.log("[Web] http://127.0.0.1:" + server.address().port + "/");
});

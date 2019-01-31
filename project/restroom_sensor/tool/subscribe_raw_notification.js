#!/usr/bin/env node

const mqtt = require("mqtt");

const client = mqtt.connect("mqtt://127.0.0.1");

client.on("connect", () => {
  console.log("connect");

  client.subscribe("sensor/restroom/raw/notification", (err) => {
    console.log("subscribe");
  });
});

client.on("message", (topic, message) => {
  console.log("message:", [topic, JSON.parse(message)]);
});

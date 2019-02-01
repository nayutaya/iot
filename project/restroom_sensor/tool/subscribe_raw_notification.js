#!/usr/bin/env node

const mqtt = require("mqtt");

const MQTT_SERVER_URL = process.env.MQTT_SERVER_URL;
console.log("MQTT_SERVER_URL:", MQTT_SERVER_URL);

const client = mqtt.connect(MQTT_SERVER_URL);

client.on("connect", () => {
  console.log("connect");

  client.subscribe("sensor/restroom/raw/notification", (err) => {
    console.log("subscribe");
  });
});

client.on("message", (topic, message) => {
  console.log("message:", [topic, JSON.parse(message)]);
});

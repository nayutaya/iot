#!/usr/bin/env node

const mqtt = require("mqtt");

const MQTT_SERVER_URL = process.env.MQTT_SERVER_URL;
console.log("MQTT_SERVER_URL:", MQTT_SERVER_URL);

const client = mqtt.connect(MQTT_SERVER_URL);

client.on("connect", () => {
  console.log("connect");

  const topic = "sensor/restroom/raw/control";
  setInterval(() => {
    const data = {
      Command: "SET_LED",
      Color: {
        Red:   Math.floor(Math.random() * 255),
        Green: Math.floor(Math.random() * 255),
        Blue:  Math.floor(Math.random() * 255),
      },
    };
    const message = JSON.stringify(data);
    client.publish(topic, message);
    console.log("publish:", message);
  }, 1000);
});

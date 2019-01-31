#!/usr/bin/env node

const mqtt = require("mqtt");

const client = mqtt.connect("mqtt://127.0.0.1");

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

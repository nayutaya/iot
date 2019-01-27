#!/usr/bin/env node

const mqtt = require("mqtt");

const client = mqtt.connect("mqtt://127.0.0.1");

client.on("connect", () => {
  console.log("connect");

  const topic = "test";
  setInterval(() => {
    const message = Date.now().toString();
    client.publish(topic, message);
    console.log("publish:", message);
  }, 1000);
});

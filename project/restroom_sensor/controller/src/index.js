#!/usr/bin/env node

const mqtt = require("mqtt");

// TODO: MQTTサーバのアドレスを環境変数から取得する。
const client = mqtt.connect("mqtt://127.0.0.1");

const NOTIFICATION_TOPIC = "sensor/restroom/raw/notification";
const CONTROL_TOPIC      = "sensor/restroom/raw/control";


client.on("connect", () => {
  console.log("connect");

  client.subscribe(NOTIFICATION_TOPIC, (err) => {
    console.log("subscribe");
  });

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
    client.publish(CONTROL_TOPIC, message);
    console.log("publish:", message);
  }, 1000);
});

client.on("message", (topic, message) => {
  if ( topic === NOTIFICATION_TOPIC ) {
    console.log("message:", [topic, JSON.parse(message)]);
  }
});

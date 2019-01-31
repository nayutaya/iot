#!/usr/bin/env node

const mqtt = require("mqtt");

// TODO: MQTTサーバのアドレスを環境変数から取得する。
const client = mqtt.connect("mqtt://127.0.0.1");

const NOTIFICATION_TOPIC = "sensor/restroom/raw/notification";

client.on("connect", () => {
  console.log("connect");

  client.subscribe(NOTIFICATION_TOPIC, (err) => {
    console.log("subscribe");
  });
});

client.on("message", (topic, message) => {
  if ( topic === NOTIFICATION_TOPIC ) {
    console.log("message:", [topic, JSON.parse(message)]);
  }
});

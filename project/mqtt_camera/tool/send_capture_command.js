#!/usr/bin/env node

const mqtt = require("mqtt");

const MQTT_SERVER_URL = process.env.MQTT_SERVER_URL;
const MQTT_TOPIC      = process.env.MQTT_TOPIC;
console.log("MQTT_SERVER_URL:", MQTT_SERVER_URL);
console.log("MQTT_TOPIC:", MQTT_TOPIC);

const client = mqtt.connect(MQTT_SERVER_URL);

client.on("connect", () => {
  console.log("connect");

  // TODO: 撮影要求電文を作成する。
  const message = "{}";
  client.publish(MQTT_TOPIC, message);
  console.log("publish:", message);

  client.end();
});

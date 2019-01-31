#!/usr/bin/env node

const mqtt = require("mqtt");

// TODO: MQTTサーバのアドレスを環境変数から取得する。
const client = mqtt.connect("mqtt://127.0.0.1");

const NOTIFICATION_TOPIC = "sensor/restroom/raw/notification";
const CONTROL_TOPIC      = "sensor/restroom/raw/control";


client.on("connect", () => {
  console.log("[MQTT] connect");

  client.subscribe(NOTIFICATION_TOPIC, (err) => {
    console.log("[MQTT] subscribe");
  });
});

client.on("message", (topic, message) => {
  if ( topic === NOTIFICATION_TOPIC ) {
    const notificationMessage = JSON.parse(message);
    console.log("[MQTT] notificationMessage:", notificationMessage);

    // TODO: 閾値を定数化する。
    var color;
    if ( notificationMessage.LightSensorValue >= 1024 ) {
      color = {Red: 255, Green: 0, Blue: 0};
    } else {
      color = {Red: 0, Green: 255, Blue: 0};
    }

    const controlMessage = JSON.stringify({
      Command: "SET_LED",
      Color: color,
    });
    client.publish(CONTROL_TOPIC, controlMessage);
    console.log("[MQTT] controlMessage:", controlMessage);
  }
});

#!/usr/bin/env node

const MQTT_SERVER_URL  = process.env.MQTT_SERVER_URL;
const MQTT_STATE_TOPIC = "sensor/restroom/state";
const WEB_API_HOST     = "0.0.0.0";
const WEB_API_PORT     = 8080;
console.log("MQTT_SERVER_URL:", MQTT_SERVER_URL);
console.log("MQTT_STATE_TOPIC:", MQTT_STATE_TOPIC);
console.log("WEB_API_HOST:", WEB_API_HOST);
console.log("WEB_API_PORT:", WEB_API_PORT);

const {mqttClient, stateSubject} = require("./mqtt")({
  mqttServerUrl: MQTT_SERVER_URL,
  stateTopic: MQTT_STATE_TOPIC,
});

const {stateHistorySubject} = require("./controller")({stateSubject})

const {webServer} = require("./web")({
  webApiHost: WEB_API_HOST,
  webApiPort: WEB_API_PORT,
  stateSubject,
  stateHistorySubject,
});

stateSubject.subscribe((state) => {
  console.log("[Rx] state:", state);
});

stateHistorySubject.subscribe((stateHistory) => {
  // console.log("[Rx] stateHistory:", stateHistory);
});

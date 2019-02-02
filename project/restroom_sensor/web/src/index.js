#!/usr/bin/env node

const express = require("express");
const Rx      = require("rxjs");
const {
  filter,
  // map,
  scan,
} = require("rxjs/operators");

const MQTT_SERVER_URL = process.env.MQTT_SERVER_URL;
const STATE_TOPIC     = "sensor/restroom/state";
console.log("MQTT_SERVER_URL:", MQTT_SERVER_URL);
console.log("STATE_TOPIC:", STATE_TOPIC);

const {mqttClient, stateSubject} = require("./mqtt")({
  mqttServerUrl: MQTT_SERVER_URL,
  stateTopic: STATE_TOPIC,
});

const stateHistoryStream = stateSubject
  .pipe(
    filter((notificationMessage) => notificationMessage !== null),
    scan((stateHistory, state) => {
      const newStateHistory = [].concat(stateHistory);
      newStateHistory.push(state);
      // TODO: レコード数を制限する。
      // TODO: レコードの期間を制限する。
      return newStateHistory;
    }),
  )

stateSubject.subscribe((state) => {
  console.log("[Rx] state:", state);
});

stateHistoryStream.subscribe((stateHistory) => {
  console.log("[Rx] stateHistory:", stateHistory);
});

const app = express();

app.get("/state.json", (req, res, next) => {
  res.json({
    Time: new Date().getTime(),
    State: stateSubject.value,
  });
});

const server = app.listen(8080, () => {
  console.log("[Web] http://127.0.0.1:" + server.address().port + "/");
});

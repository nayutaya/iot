#!/usr/bin/env node

const mqtt = require("mqtt");

const Rx = require("rxjs");
const {
  filter,
  scan,
} = require("rxjs/operators");

// TODO: MQTTサーバのアドレスを環境変数から取得する。
const client = mqtt.connect("mqtt://127.0.0.1");

const NOTIFICATION_TOPIC = "sensor/restroom/raw/notification";
const CONTROL_TOPIC      = "sensor/restroom/raw/control";
const LIGHT_SENSOR_THRESHOLD = 1024;
const COLOR_BUSY = {Red: 255, Green: 0, Blue: 0};
const COLOR_FREE = {Red: 0, Green: 255, Blue: 0};

client.on("connect", () => {
  console.log("[MQTT] connect");

  client.subscribe(NOTIFICATION_TOPIC, (err) => {
    console.log("[MQTT] subscribe");
  });
});

const notificationMessageSubject = new Rx.BehaviorSubject(null);
client.on("message", (topic, message) => {
  if ( topic === NOTIFICATION_TOPIC ) {
    const notificationMessage = JSON.parse(message);
    console.log("[MQTT] notificationMessage:", notificationMessage);
    notificationMessageSubject.next(notificationMessage);
  }
});

const initialState = {
  LightSensorValue: 0,
  NumberOfPyroelectricSensorInterrupts: 0,
  CurrentTime: 0,
  IsLightOn: false,
  LastLightTurnedOnTime: 0,
  LastLightTurnedOffTime: 0,
  LastDetectedHumanTime: 0,
};
const stateStream = notificationMessageSubject
  .pipe(
    filter((message) => message !== null),
    scan((previousState, message) => {
      const currentState = Object.assign({}, previousState);
      currentState.LightSensorValue                     = message.LightSensorValue;
      currentState.NumberOfPyroelectricSensorInterrupts = message.NumberOfPyroelectricSensorInterrupts;
      currentState.CurrentTime                          = new Date().getTime();
      currentState.IsLightOn                            = (currentState.LightSensorValue >= LIGHT_SENSOR_THRESHOLD);

      if ( currentState.IsLightOn && !previousState.IsLightOn ) {
        currentState.LastLightTurnedOnTime = currentState.CurrentTime;
      }
      if ( !currentState.IsLightOn && previousState.IsLightOn ) {
        currentState.LastLightTurnedOffTime = currentState.CurrentTime;
      }
      if ( currentState.NumberOfPyroelectricSensorInterrupts != previousState.NumberOfPyroelectricSensorInterrupts ) {
        currentState.LastDetectedHumanTime = currentState.CurrentTime;
      }

      return currentState;
    }, initialState),
  );

stateStream.subscribe((state) => {
  console.log("[Rx] stateStream:", state);
});

stateStream.subscribe((state) => {
  const controlMessage = JSON.stringify({
    Command: "SET_LED",
    Color: (state.IsLightOn ? COLOR_BUSY : COLOR_FREE),
  });
  console.log("[MQTT] controlMessage:", controlMessage);
  client.publish(CONTROL_TOPIC, controlMessage);
});

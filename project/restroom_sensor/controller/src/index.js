#!/usr/bin/env node

const mqtt = require("mqtt");

const Rx = require("rxjs");
const {
  filter,
  map,
  scan,
} = require("rxjs/operators");

// TODO: MQTTサーバのアドレスを環境変数から取得する。
const client = mqtt.connect("mqtt://127.0.0.1");

const NOTIFICATION_TOPIC = "sensor/restroom/raw/notification";
const CONTROL_TOPIC      = "sensor/restroom/raw/control";
const LIGHT_SENSOR_THRESHOLD = 1024;
const COLOR_BUSY    = {Red: 255, Green: 0, Blue: 0};
const COLOR_FREE    = {Red: 0, Green: 255, Blue: 0};
const COLOR_UNKNOWN = {Red: 255, Green: 255, Blue: 0};

client.on("connect", () => {
  console.log("[MQTT] connect");

  client.subscribe(NOTIFICATION_TOPIC, (err) => {
    console.log("[MQTT] subscribe");
  });
});

const notificationMessageSubject = new Rx.BehaviorSubject(null);
client.on("message", (topic, message) => {
  if ( topic === NOTIFICATION_TOPIC ) {
    notificationMessageSubject.next(JSON.parse(message));
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
    filter((notificationMessage) => notificationMessage !== null),
    scan((previousState, notificationMessage) => {
      const currentState = Object.assign({}, previousState);
      currentState.LightSensorValue                     = notificationMessage.LightSensorValue;
      currentState.NumberOfPyroelectricSensorInterrupts = notificationMessage.NumberOfPyroelectricSensorInterrupts;
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

const controlMessageStream = stateStream
  .pipe(
    map((state) => {
      return {
        Command: "SET_LED",
        Color:
          (state.IsLightOn
            ? (
                   (state.LastLightTurnedOnTime + 1000 * 60 * 5 < state.CurrentTime)
                && (state.LastDetectedHumanTime + 1000 * 60 * 5 < state.CurrentTime)
                ? COLOR_UNKNOWN
                : COLOR_BUSY)
            : COLOR_FREE),
      };
    }),
  );
controlMessageStream.subscribe((controlMessage) => {
  client.publish(CONTROL_TOPIC, JSON.stringify(controlMessage));
});

notificationMessageSubject.subscribe((notificationMessage) => {
  console.log("[Rx] notificationMessage:", notificationMessage);
});

stateStream.subscribe((state) => {
  console.log("[Rx] state:", state);
});

controlMessageStream.subscribe((controlMessage) => {
  console.log("[Rx] controlMessage:", controlMessage);
});

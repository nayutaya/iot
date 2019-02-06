
const mqtt = require("mqtt");
const Rx   = require("rxjs");

module.exports = ({mqttServerUrl, stateTopic}) => {
  const mqttClient   = mqtt.connect(mqttServerUrl);
  const stateSubject = new Rx.BehaviorSubject(null);

  mqttClient.on("connect", () => {
    console.log("[MQTT] connect");

    mqttClient.subscribe(stateTopic, (err) => {
      console.log("[MQTT] subscribe:", stateTopic);
    });
  });

  mqttClient.on("message", (topic, message) => {
    if ( topic === stateTopic ) {
      stateSubject.next(JSON.parse(message));
    }
  });

  return {mqttClient, stateSubject}
};


const express = require("express");
const cors    = require("cors");

module.exports = ({webApiHost, webApiPort, stateSubject, stateHistorySubject}) => {
  const app = express();
  const expressWs = require("express-ws")(app);

  app.use(cors());
  app.use(express.static(__dirname + "/../client/build"));

  app.get("/state.json", (req, res, next) => {
    res.json({
      Time: new Date().getTime(),
      State: stateSubject.value,
    });
  });

  app.get("/state/history.json", (req, res, next) => {
    res.json({
      Time: new Date().getTime(),
      StateHistory: stateHistorySubject.value,
    });
  });

  app.ws("/", (ws, req) => {
    ws.on("message", (msg) => {
      // nop
    });

    ws.send(JSON.stringify(stateHistorySubject.value));
  });

  stateSubject.subscribe((state) => {
    const json = JSON.stringify([state]);
    expressWs.getWss().clients.forEach((client) => {
      client.send(json);
    });
  });

  const webServer = app.listen(webApiPort, webApiHost, () => {
    const address = webServer.address();
    console.log("[Web] http://" + address.address + ":" + address.port + "/");
  });

  return {webServer};
};

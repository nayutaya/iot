#!/usr/bin/env node

const express = require("express");

const app = express();

app.get("/", (req, res, next) => {
  res.json({
    Time: new Date().getTime(),
  });
});

const server = app.listen(8080, () => {
  console.log("[Web] http://127.0.0.1:" + server.address().port + "/");
});

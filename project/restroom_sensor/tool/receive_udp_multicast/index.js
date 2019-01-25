#!/usr/bin/env node

const dgram = require("dgram");

const port    = 10000;
const address = "224.0.0.42";

const socket = dgram.createSocket({type: "udp4", reuseAddr: true});
socket.bind(port);
socket.on("listening", () => {
  console.log("listening");
  socket.addMembership(address);
});
socket.on("message", (message, rinfo) => {
  console.log(["message", message, rinfo]);
});

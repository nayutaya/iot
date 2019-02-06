import React from "react";
import ReactDOM from "react-dom";

import App from "./App";

const REACT_APP_WS_URL        = process.env.REACT_APP_WS_URL;
const REACT_APP_LOCATION_NAME = process.env.REACT_APP_LOCATION_NAME

const wsUrl        = (REACT_APP_WS_URL        != null && REACT_APP_WS_URL        !== "" ? REACT_APP_WS_URL        : window.location.href.replace("http://", "ws://"));
const locationName = (REACT_APP_LOCATION_NAME != null && REACT_APP_LOCATION_NAME !== "" ? REACT_APP_LOCATION_NAME : "-");
console.log("locationName:", locationName);

ReactDOM.render(<App wsUrl={wsUrl} locationName={locationName} />, document.getElementById("root"));

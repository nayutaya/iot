import React from "react";
import ReactDOM from "react-dom";

import App from "./App";

const REACT_APP_WS_URL = process.env.REACT_APP_WS_URL;
const wsUrl = (REACT_APP_WS_URL != null ? REACT_APP_WS_URL : window.location.href.replace("http://", "ws://"));

ReactDOM.render(<App wsUrl={wsUrl}/>, document.getElementById("root"));

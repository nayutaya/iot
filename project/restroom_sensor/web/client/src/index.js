import React from "react";
import ReactDOM from "react-dom";

import App from "./App";

const wsUrl = "ws://localhost:8080/";
// const wsUrl = window.location.href.replace("http://", "ws://");

ReactDOM.render(<App wsUrl={wsUrl}/>, document.getElementById("root"));

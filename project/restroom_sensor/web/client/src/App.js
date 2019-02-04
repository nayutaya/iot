import React, { Component } from "react";
import "./App.css";

import _ from "lodash";
import Websocket from "react-websocket";

export default class App extends Component {
  constructor(props) {
    super(props);
    this.state = {
      stateHistory: [],
    };
  }

  componentDidMount() {
    console.log("componentDidMount");
  }

  componentWillUnmount() {
    console.log("componentWillUnmount");
  }

  onMessage(data) {
    const stateHistory =
        _(this.state.stateHistory)
          .concat(JSON.parse(data))
          .sortBy((s) => s.CurrentTime)
          .reverse()
          .slice(0, 100)
          .reverse()
          .value();
    // console.log("stateHistory:", stateHistory);
    this.setState({stateHistory});
  }

  render() {
    return (
      <div className="App">
        App
        <Websocket url="ws://localhost:8080/" onMessage={(data) => this.onMessage(data)}/>
      </div>
    );
  }
}

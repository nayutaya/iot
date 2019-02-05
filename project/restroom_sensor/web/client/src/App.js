import React, { Component } from "react";
import "./App.css";

import _ from "lodash";
import WebSocket from "react-websocket";

import CssBaseline from "@material-ui/core/CssBaseline";
import Grid from "@material-ui/core/Grid";

export default class App extends Component {
  constructor(props) {
    super(props);
    this.state = {
      stateHistory: [],
      currentState: null,
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
          .slice(0, 10)
          .value();
    // console.log("stateHistory:", stateHistory);
    const currentState = stateHistory[0];
    this.setState({stateHistory, currentState});
  }

  render() {
    return (
      <div className="App">
        <WebSocket url="ws://localhost:8080/" onMessage={(data) => this.onMessage(data)}/>
        <CssBaseline />
        <Grid container spacing={16}>
          <Grid item xs={6}>
            {(this.state.currentState == null ? null :
              <>
                <div>CurrentTime: {this.state.currentState.CurrentTime}</div>
                <div>State: {this.state.currentState.State}</div>
              </>
            )}
          </Grid>
          <Grid item xs={6}>
            <table>
              <thead>
                <tr>
                  <th>CurrentTime</th>
                  <th>State</th>
                </tr>
              </thead>
              <tbody>
                {this.state.stateHistory.map((state) => {
                  return (
                    <tr key={state.CurrentTime}>
                      <td>{state.CurrentTime}</td>
                      <td>{state.State}</td>
                    </tr>
                  );
                })}
              </tbody>
            </table>
          </Grid>
        </Grid>
      </div>
    );
  }
}

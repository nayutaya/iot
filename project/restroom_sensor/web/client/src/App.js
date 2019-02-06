import _ from "lodash";
import moment from "moment";
import React, { Component } from "react";
import WebSocket from "react-websocket";

import Card from "@material-ui/core/Card";
import CardActions from "@material-ui/core/CardActions";
import CardContent from "@material-ui/core/CardContent";
import CardHeader from "@material-ui/core/CardHeader";
import Collapse from "@material-ui/core/Collapse";
import CssBaseline from "@material-ui/core/CssBaseline";
import ExpandMoreIcon from "@material-ui/icons/ExpandMore";
import Grid from "@material-ui/core/Grid";
import IconButton from "@material-ui/core/IconButton";
import Typography from "@material-ui/core/Typography";

export default class App extends Component {
  constructor(props) {
    super(props);
    this.state = {
      stateHistory: [],
      currentState: null,
      expanded: false,
    };
  }

  onMessage(data) {
    const stateHistory =
        _(this.state.stateHistory)
          .concat(JSON.parse(data))
          .sortBy((s) => -s.CurrentTime)
          .slice(0, 10)
          .value();
    this.setState({
      stateHistory,
      currentState: stateHistory[0],
    });
  }

  render() {
    const formatTime = (time) => {
      return moment(new Date(time)).format("YYYY/MM/DD HH:mm:ss");
    };
    const getState = (state) => {
      const stateTable = {
        BUSY:    {color: "#ff9999", short: "使用中", message: "トイレは使用中です"},
        FREE:    {color: "#b3ffb3", short: "空き",   message: "トイレは空いています"},
        UNKNOWN: {color: "#ff9900", short: "不明",   message: "トイレの照明が点きっぱなしかも？"},
      };
      return (stateTable.hasOwnProperty(state) ? stateTable[state] : stateTable.UNKNOWN);
    };
    const {currentState} = this.state;

    return (
      <div className="App">
        <WebSocket url={this.props.wsUrl} onMessage={(data) => this.onMessage(data)}/>
        <CssBaseline />

        <Grid container direction="row" justify="center" alignItems="flex-start" spacing={16}>
          <Grid item xs={4}>
            <Card>
              <CardHeader
                  title={this.props.locationName}
                  subheader={currentState == null ? "-" : formatTime(currentState.CurrentTime)} />
              <CardContent>
                {(currentState == null ? null :
                  <Typography component="p">
                    状態: <span style={{backgroundColor: getState(currentState.State).color}}>{getState(currentState.State).message}</span>
                  </Typography>
                )}
              </CardContent>
              <CardActions disableActionSpacing>
                <IconButton
                    onClick={() => this.setState({expanded: !this.state.expanded})}
                    aria-expanded={this.state.expanded}
                    aria-label="Show more">
                  <ExpandMoreIcon />
                </IconButton>
              </CardActions>
              <Collapse in={this.state.expanded} timeout="auto" unmountOnExit>
                <CardContent>
                  <table>
                    <thead>
                      <tr>
                        <th>日時</th>
                        <th>状態</th>
                      </tr>
                    </thead>
                    <tbody>
                      {this.state.stateHistory.map((state) => {
                        return (
                          <tr key={state.CurrentTime} style={{backgroundColor: getState(state.State).color}}>
                            <td>{formatTime(state.CurrentTime)}</td>
                            <td>{getState(state.State).short}</td>
                          </tr>
                        );
                      })}
                    </tbody>
                  </table>
                </CardContent>
              </Collapse>
            </Card>
          </Grid>
        </Grid>
      </div>
    );
  }
}

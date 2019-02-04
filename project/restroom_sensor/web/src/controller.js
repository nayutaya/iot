
const _  = require("lodash");
const Rx = require("rxjs");
const {
  filter,
  scan,
} = require("rxjs/operators");

module.exports = ({stateSubject}) => {
  const stateHistorySubject = new Rx.BehaviorSubject([]);
  stateSubject
    .pipe(
      filter((notificationMessage) => notificationMessage !== null),
      scan((stateHistory, state) => {
        return _(stateHistory)
          .concat([state])
          .sortBy((s) => s.CurrentTime)
          .reverse()
          .slice(0, 100)
          .reverse()
          .value();
      }, []),
    )
    .subscribe((stateHistory) => {
      stateHistorySubject.next(stateHistory);
    });

  return {stateHistorySubject};
};

module.exports = class Timer {
  constructor() {
    this.time = Date.now();
  }

  wait(period) {
    period = Math.floor(period / 1000); // us -> ms
    this.time += period;
    let adjust = Date.now() - this.time;
    if(period < adjust) adjust = period;
    return new Promise(resolve => setTimeout(resolve, period - adjust));
  }

  update() {
    this.time = Date.now();
  }
}
'use strict'

module.exports = class Timer {
  constructor() {
    this.time = Date.now();
  }

  wait(period) {
    period = Math.floor(period / 1000); // us -> ms
    this.time += period;
    const diff = Date.now() - this.time;
    return new Promise(resolve => setTimeout(resolve, period - diff));
  }

  update() {
    this.time = Date.now();
  }
}
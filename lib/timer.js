'use strict'

module.exports = class Timer {
  constructor() {
    this.time = Date.now();
  }

  wait(period) {
    period = Math.floor(period / 1000); // us -> ms
    this.time += period;
    const diff =  this.time - Date.now();
    return new Promise(resolve => setTimeout(resolve, diff));
  }

  update() {
    this.time = Date.now();
  }
}
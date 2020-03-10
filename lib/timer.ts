export class Timer {
  time: number;
  constructor() {
    this.time = Date.now()
  }

  wait(period: number) {
    period = Math.floor(period / 1000) // us -> ms
    this.time += period
    let adjust = Date.now() - this.time
    return new Promise(resolve => setTimeout(resolve, Math.max(period - adjust, 0)))
  }

  update() {
    this.time = Date.now()
  }

  get dt() {
    return Date.now() - this.time
  }
}
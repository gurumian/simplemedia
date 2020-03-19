export class Timer {
  private time: number;
  constructor() {
    this.time = Date.now()
  }

  async wait(period: number): Promise<() => void> {
    period = Math.round(period / 1000) // us -> ms
    this.time += period
    let d = period - this.dt
    return new Promise(resolve => setTimeout(resolve, Math.max(d, 0)))
  }

  update(): void {
    this.time = Date.now()
  }

  get dt(): number {
    return Date.now() - this.time
  }
}
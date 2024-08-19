import time
import threading
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from gpio import GPIO

MONITOR_PINS = [0, 1, 2, 3, 4]
MONITOR_PINS = [0, 1, 2, 3, 4]

POLL_INTERVAL = 0.5  # 500ms polling interval

class SignalAnalyzer:
    def __init__(self, gpio):
        self.gpio = gpio
        self.pin_states = {pin: [] for pin in MONITOR_PINS}
        self.timestamps = []
        self.polling_thread = None
        self.polling_active = False

    def poll_pins(self, gpio):
        """Polls the GPIO pins at a fixed rate and stores the states."""
        while self.polling_active:
            gpio._read_pins() # refresh gpio module

            current_time = time.time()
            self.timestamps.append(current_time)

            for pin in MONITOR_PINS:
                # Read the actual state from GPIO pin (real pin states)
                pin_state = self.gpio.read_pin(pin)  # This returns 0 or 1
                self.pin_states[pin].append(pin_state)

            max_data_points = 100
            if len(self.timestamps) > max_data_points:
                self.timestamps = self.timestamps[-max_data_points:]
                for pin in MONITOR_PINS:
                    self.pin_states[pin] = self.pin_states[pin][-max_data_points:]

            time.sleep(POLL_INTERVAL)

    def start_polling(self, gpio):
        """Starts the polling in a separate thread."""
        if not self.polling_active:
            self.polling_active = True
            self.polling_thread = threading.Thread(target=self.poll_pins, args=(gpio,), daemon=True)
            self.polling_thread.start()

    def stop_polling(self):
        """Stops the polling thread."""
        if self.polling_active:
            self.polling_active = False
            if self.polling_thread:
                self.polling_thread.join()

class SignalAnalyzerGUI:
    """A simplified GUI with plotting functionality."""
    def __init__(self, analyzer: SignalAnalyzer):
        self.analyzer = analyzer
        self.fig, self.axes = plt.subplots(len(MONITOR_PINS), 1, figsize=(10, 5), sharex=True)
        if len(MONITOR_PINS) == 1:
            self.axes = [self.axes]

    def setup_plot(self):
        """Sets up the initial plot configuration."""
        for i, pin in enumerate(MONITOR_PINS):
            self.axes[i].set_ylim(-0.1, 1.1)
            self.axes[i].set_ylabel(f"Pin {pin} State")
            self.axes[i].set_title(f"Pin {pin} Signal")

        self.axes[-1].set_xlabel("Time (seconds)")

    def update_plot(self, frame):
        """Updates the plot with new data."""
        for i, pin in enumerate(MONITOR_PINS):
            self.axes[i].cla()  # Clear previous plot
            self.axes[i].set_ylim(-0.1, 1.1)
            self.axes[i].set_ylabel(f"Pin {pin} State")
            self.axes[i].set_title(f"Pin {pin} Signal")
            self.axes[i].plot(self.analyzer.timestamps, self.analyzer.pin_states[pin], label=f"Pin {pin}")

        self.axes[-1].set_xlabel("Time (seconds)")

    def toggle_polling(self, gpio):
        """Toggles between starting and stopping polling."""
        if self.analyzer.polling_active:
            self.analyzer.stop_polling()
        else:
            self.analyzer.start_polling(gpio)

    def show_plot(self):
        """Display the plot."""
        self.setup_plot()
        ani = FuncAnimation(self.fig, self.update_plot, blit=False, interval=500)  # 500ms refresh
        plt.show()

def main():
    # Initialize GPIO object
    gpio = GPIO()  # Replace with the actual GPIO object

    # Create SignalAnalyzer instance and SignalAnalyzerGUI
    analyzer = SignalAnalyzer(gpio)
    gui = SignalAnalyzerGUI(analyzer)
    gui.toggle_polling(gpio)  # Start polling

    # Start the real-time plot update
    gui.show_plot()

if __name__ == "__main__":
    main()

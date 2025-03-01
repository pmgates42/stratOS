import tkinter as tk
from tkinter import messagebox
from gpio import GPIO, NUMBER_OF_GPIO_PINS  # Assuming the GPIO class is saved in gpio.py

class GPIOControlApp:
    def __init__(self, root, gpio: GPIO):
        self.root = root
        self.gpio = gpio
        self.root.title("GPIO Pin Control")

        self.create_widgets()

    def create_widgets(self):
        # Display current pin states
        self.pin_state_label = tk.Label(self.root, text="Pin States", font=("Arial", 16))
        self.pin_state_label.grid(row=0, column=0, columnspan=3, pady=10)

        # Create button for each pin (toggle)
        self.pin_buttons = []
        for i in range(NUMBER_OF_GPIO_PINS):
            pin_button = tk.Button(self.root, text=f"Pin {i}", width=14, command=lambda i=i: self.toggle_pin(i))
            pin_button.grid(row=i+1, column=0)
            self.pin_buttons.append(pin_button)

            pin_state_label = tk.Label(self.root, text="State", width=14)
            pin_state_label.grid(row=i+1, column=1)
            self.update_pin_state(i)

            mode_button = tk.Button(self.root, text="Set Mode", width=10, command=lambda i=i: self.set_pin_mode(i))
            mode_button.grid(row=i+1, column=2)

    def toggle_pin(self, pin: int):
        """Toggles the state of a specific pin."""
        if self.gpio.pins[pin][0] == self.gpio.OUTPUT_PIN:
            current_value = self.gpio.read_pin(pin)
            new_value = 1 if current_value == 0 else 0
            self.gpio.write_pin(pin, new_value)
            self.update_pin_state(pin)
        else:
            messagebox.showwarning("Invalid Operation", f"Pin {pin} is not an OUTPUT pin!")

    def set_pin_mode(self, pin: int):
        """Allows the user to set the mode of the pin."""
        def confirm_set_mode(mode):
            self.gpio.set_pin_mode(pin, mode)
            self.update_pin_state(pin)
            mode_window.destroy()

        mode_window = tk.Toplevel(self.root)
        mode_window.title(f"Set Pin {pin} Mode")
        
        mode_label = tk.Label(mode_window, text=f"Choose mode for Pin {pin}:", font=("Arial", 12))
        mode_label.grid(row=0, column=0, padx=10, pady=10)

        input_button = tk.Button(mode_window, text="INPUT", width=15, command=lambda: confirm_set_mode(self.gpio.INPUT_PIN))
        input_button.grid(row=1, column=0, padx=10, pady=5)

        output_button = tk.Button(mode_window, text="OUTPUT", width=15, command=lambda: confirm_set_mode(self.gpio.OUTPUT_PIN))
        output_button.grid(row=2, column=0, padx=10, pady=5)

    def update_pin_state(self, pin: int):
        """Updates the state label of the specified pin."""
        mode, value = self.gpio.pins[pin]
        mode_str = "INPUT" if mode == self.gpio.INPUT_PIN else "OUTPUT" if mode == self.gpio.OUTPUT_PIN else "UNCONFIGURED"
        value_str = "HIGH" if value == 1 else "LOW"

        self.pin_buttons[pin].config(text=f"Pin {pin}: {mode_str}")
        self.pin_buttons[pin].grid(row=pin+1, column=0)
        label = self.root.grid_slaves(row=pin+1, column=1)[0]
        label.config(text=value_str)

def main():
    gpio = GPIO()
    root = tk.Tk()
    app = GPIOControlApp(root, gpio)
    root.mainloop()

if __name__ == "__main__":
    main()

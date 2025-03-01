import os
from typing import List, Tuple

PIN_FILE_FNAME = "../../build/sim/sim_gpio_pin_states.state"
NUMBER_OF_GPIO_PINS = 50

class GPIO:
    UNCONFIGURED_PIN = 0
    OUTPUT_PIN = 1
    INPUT_PIN = 2

    def __init__(self):
        self.pins = [(self.UNCONFIGURED_PIN, 0) for _ in range(NUMBER_OF_GPIO_PINS)]
        self._read_pins()
    
    def _read_pins(self):
        """Reads the GPIO pin states from the file."""
        if not os.path.exists(PIN_FILE_FNAME):
            return
        
        with open(PIN_FILE_FNAME, "r") as f:
            lines = f.readlines()
        
        for i, line in enumerate(lines):
            if i >= NUMBER_OF_GPIO_PINS:
                break
            try:
                io_state, value = map(int, line.split())
                self.pins[i] = (io_state, value)
            except ValueError:
                pass  # Ignore malformed lines
    
    def _write_pins(self):
        """Writes the GPIO pin states to the file."""
        with open(PIN_FILE_FNAME, "w") as f:
            for io_state, value in self.pins:
                f.write(f"{io_state} {value}\n")
    
    def set_pin_mode(self, pin: int, mode: int):
        """Sets the mode of a given pin (input/output)."""
        if 0 <= pin < NUMBER_OF_GPIO_PINS:
            self.pins[pin] = (mode, self.pins[pin][1])
            self._write_pins()
    
    def write_pin(self, pin: int, value: int):
        """Writes a value (0 or 1) to a given pin."""
        if 0 <= pin < NUMBER_OF_GPIO_PINS and self.pins[pin][0] == self.OUTPUT_PIN:
            self.pins[pin] = (self.OUTPUT_PIN, 1 if value else 0)
            self._write_pins()
    
    def read_pin(self, pin: int) -> int:
        """Reads the value of a given pin."""
        if 0 <= pin < NUMBER_OF_GPIO_PINS:
            return self.pins[pin][1]
        return 0
    
    def get_all_pins(self) -> List[Tuple[int, int]]:
        """Returns all pin states as a list of tuples (mode, value)."""
        return self.pins

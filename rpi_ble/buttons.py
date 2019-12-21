
"""
    General class for representing a button on the Joy Con.
    Although we will holding the JoyCon on its side we will
    use naming consistent with 
    https://en-americas-support.nintendo.com/app/answers/detail/a_id/22634/~/joy-con-controller-diagram
    
    For simplicity we will refer to all buttons by their names on the RIGHT Joy Con. This is because
    both joycons are basically symmetric (and directional pad may get confusing).

    Note when refering to the direction of the directional stick we will refer to the direction towards
    the buttons as right.
"""

"""
    A button state is essentially a mapping of an output value to a button name.
"""
class ButtonState:

    """
        A button state must be defined by the value it will output
    """
    def __init__(self, output, name):
        assert(type(output) == int)
        assert(type(name) == str and len (name) > 0)
        self.output = output
        self.name = name

    def __str__(self):
        return self.name

    def __repr__(self):
        return "Button State with name {} that outputs {} to the Kobuki" \
                .format (self.name, self.output)



"""
    A button is defined by a list of values that map to button states, the names/outputs
    cannot repeat for an individual button and there must be a button with name "NOT PRESSED".

    Additionally a button also contains 2 lists of bits, which correspond to the bits in the packet
    sent by the Joy Con which contains the state information and the bits in the output BLE packet it occupies. 
    Because the Joy Con sends all the info for a particular button in the same byte all bits listed must 
    be contiguous and in the same byte.

    Additionally we define bits relative to the 12 byte packet sent by the Joy Con. We define the MSB
    of the 0th byte as bit 0 and the LSB of the 11 byte as bit 95.

    The output bits are similar except we are between bytes 0-1 and output bits are allowed to overlap
    for mutually exclusive buttons between the left and right Joy Con.
"""
class Button:
    def __init__(self, input_bit_list, output_bit_list, button_states_map, name):
        self.input_byte_num, self.input_mask, self.input_shift = self._validate_bit_list (input_bit_list, 11)
        self.output_byte_num, _, self.output_shift = self._validate_bit_list (output_bit_list, 1)
        self.mapping, self.state = self._validate_map (button_states_map)
        self.name = self._validate_name (name)

    """
        Takes in a list of bits, confirms that all are contiguous,
        refer to the same byte, and that the byte is in the first 11 bytes.

        Returns a tuple consisting of the byte that contains the value,
        the mask for extracting it, and the amount to shift to get the value.
    """
    def _validate_bit_list(self, bit_list, max_byte):
        # Check that we passed in a non-zero len list
        assert(isinstance(bit_list, list))
        assert(len(bit_list) > 0)
        bit_set = set()
        # Check that every bit is an int
        # and is unique
        for i in bit_list:
            assert(type(i) == int)
            bit_set.add (i)
        assert(len(bit_set) == len(bit_list))
        # Check that all bits are in the same byte
        byte_list = [i // 8 for i in bit_list]
        # Select the byte referred to
        byte_num = byte_list[0]
        # Make sure we are within the first 12 bytes
        assert (0 <= byte_num <= max_byte)
        for i in byte_list[1:]:
            assert(i == byte_num)
        # Check that all bits are contiguous. We don't care about
        # the order given.
        ones_list = [7 - (bit_val % 8) for bit_val in bit_list]
        ones_list.sort(reverse=True)
        assert (len(ones_list) == (ones_list[0] - ones_list[-1] + 1))
        # Select our shift value
        shift_value = ones_list[-1]
        # Construct our mask
        mask = 0
        for on_bit in ones_list:
            mask += (1 << on_bit)
        return byte_num, mask, shift_value

    """
        Takes in a mapping of input values to each state. Confirms all states
        are unique, the inputs are allowable, and at least one has name
        "NOT PRESSED".

        Returns a tuple of the mapping passed in and the "NOT PRESSED" state,
        so the button can be initialized to that state.
    """
    def _validate_map(self, states_map):
        assert(type(states_map) == dict and len(states_map) > 0)
        # Determine the max value possible for the input
        max_value = self.input_mask >> self.input_shift
        # Define the not pressed state
        not_pressed_state = None
        # Create a set for the possible states and inputs
        inputs = set()
        states = set()
        for input_val, state in states_map.items():
            assert(type(input_val) == int)
            assert(isinstance(state, ButtonState))
            assert(0 <= input_val <= max_value)
            if str(state) == "NOT PRESSED":
                not_pressed_state = state
            inputs.add(input_val)
            states.add(str(state))
        assert(not_pressed_state is not None)
        assert(len(inputs) == len(states_map))
        assert(len(states) == len(states_map))
        return states_map, not_pressed_state

    """
        Takes in a name and confirms it is not an empty string.

        Returns the name.
    """
    def _validate_name(self, name):
        assert(type(name) == str and len (name) > 0)
        return name
    
    def __str__(self):
        return "Button {} in state {}" \
                .format(self.name, self.state)

    def __repr__(self):
        return ("Button {}. It contains the following map of inputs to outputs\n {}.\n" \
                + "It checks its state with the mask {} on byte {}, shifting by {} bits.\n" \
                + "It outputs its state on byte {}, shifting by {} bits.\n" \
                + "The button is currently in state {}") \
                .format (self.name, repr(self.mapping), hex(self.input_mask), \
                    self.input_byte_num, self.input_shift, self.output_byte_num, self.output_shift, self.state)

    """
        Returns the status of the button.
    """
    def return_status(self):
        return "{} is in state {}".format(self.name, self.state)

    """
        Prints the current status of the button
    """
    def display_status(self):
        # print(self.return_status())
        pass

    """
        Returns if a button is not pressed
    """
    def is_not_pressed(self):
        return str(self.state) == "NOT PRESSED"

    """
        Prints the current status of the button as long as it is not in the 
        NOT PRESSED state.

        Returns if the status was printed.
    """
    def display_if_pressed(self):
        if not self.is_not_pressed():
            self.display_status()
            return True
        return False

    """
        Takes in an input 12 byte string and updates the value of the current
        state.
    """
    def parse_next_state(self, input_msg):
        assert (len(input_msg) == 12)
        new_input = int(input_msg[self.input_byte_num] & self.input_mask) >> self.input_shift
        assert(new_input in self.mapping)
        self.state = self.mapping[new_input]

    """
        Takes in a message and appends the output in the right location.
    """
    def append_output(self, output_msg):
        assert (len(output_msg) == 2)
        added_input = (self.state.output << self.output_shift)
        # Zero out all the bits we occupy
        output_byte = int(output_msg[self.output_byte_num]) 
        # Or the bits
        output_byte = output_byte | added_input
        result = output_msg[0:self.output_byte_num] + bytearray([output_byte]) + output_msg[self.output_byte_num+1:] 
        return result


"""
    A button that only has 2 states: pressed and not pressed. It is otherwise
    the same.
"""
class ToggleButton(Button):

    def __init__(self, input_bit_list, output_bit_list, name):
        # Button map for a toggleable button
        toggleable_states = {1: ButtonState(1, "PRESSED"), 0: ButtonState(0, "NOT PRESSED")}
        super().__init__(input_bit_list, output_bit_list, toggleable_states, name)


"""
    Sample class to contain information for a particular Controller
"""
class Controller:

    def __init__(self, button_list, name):
        self.buttons = self._validate_button_list(button_list)
        self.name = self._validate_name (name)

    """
        Function that validates that all the buttons in a button
        list are not reading from overlapping input values and
        that all the names are unique.

        Returns the list of buttons
    """
    def _validate_button_list(self, button_list):
        assert(isinstance(button_list, list) and len(button_list) > 0)
        # Verify that all input bits and names are unique
        name_set = set()
        input_bits_set = set()
        for i, button in enumerate(button_list):
            assert(isinstance(button, Button))
            name_set.add(button.name)
            input_mask = button.input_mask
            input_byte_bits = 8 * button.input_byte_num
            for ctr in range(8):
                bit_offset = 7 - ctr
                if input_mask & (1 << bit_offset):
                    bit = input_byte_bits + bit_offset
                    assert (bit not in input_bits_set)
                    input_bits_set.add(bit)
        assert(len(name_set) == len(button_list))
        return button_list

    """
        Function that validates that the name is a string that is not empty

        Returns the same string.
    """
    def _validate_name(self, name):
        assert(type(name) == str and len(name) > 0)
        return name


    """
        Parses a 12 byte input message and updates the state of each button.
    """
    def parse_next_state(self, input_msg):
        for button in self.buttons:
            button.parse_next_state(input_msg)

    """
        Returns the status of all buttons.
    """
    def return_status(self):
        return [button.return_status() for button in self.buttons]

    """
        Prints the current status of all buttons.
    """
    def display_status(self):
        for button in self.buttons:
            button.display_status()

    """
        Prints the current status of buttons not in the NOT PRESSED state.

        Prints out a message indicating no buttons were pressed if none are
        displayed.
    """
    def display_all_pressed_buttons(self):
        status = False
        # print("Current State:")
        for button in self.buttons:
            new_status = button.display_if_pressed()
            status = status or new_status
        if not status:
            # print("No buttons are currently pressed")
            pass
    
    """
        Constructs the output message to send over BLE to the Kobuki. 
    """
    def get_output_message(self):
        output_msg = bytes([0, 0])
        for button in self.buttons:
            output_msg = button.append_output(output_msg)
        return bytearray([output_msg[1]] + [output_msg[0]])

"""
    Sample class to contain information for a particular JoyCon
"""
class JoyCon(Controller):

    """
        To make sure each JoyCon is different we will create a new button
        for each JoyCon.
    """
    def __init__(self):
        buttons = []
        buttons.append(ToggleButton([10], [9], "SR"))
        buttons.append(ToggleButton([11], [8], "SL"))
        buttons.append(ToggleButton([12], [13], "Y"))
        buttons.append(ToggleButton([13], [15], "B"))
        buttons.append(ToggleButton([14], [12], "X"))
        buttons.append(ToggleButton([15], [14], "A"))
        buttons.append(ToggleButton([16], [11], "RZ"))
        buttons.append(ToggleButton([17], [10], "R"))
        buttons.append(ToggleButton([18], [1], "CAPTURE"))
        buttons.append(ToggleButton([19], [1], "HOME"))
        buttons.append(ToggleButton([20], [2], "RIGHT STICK CLICK"))
        buttons.append(ToggleButton([21], [2], "LEFT STICK CLICK"))
        buttons.append(ToggleButton([22], [3], "+"))
        buttons.append(ToggleButton([23], [3], "-"))
        # Only non toggle button we have is the analog stick
        # Treating towards the button as right we get the following mapping
        stick_push_map = {0: ButtonState(0, "UP"), 1: ButtonState(1, "RIGHT-UP"), \
                2: ButtonState(2, "RIGHT"), 3: ButtonState(3, "RIGHT-DOWN"), \
                4: ButtonState(4, "DOWN"), 5: ButtonState(5, "LEFT-DOWN"), \
                6: ButtonState(6, "LEFT"), 7: ButtonState(7, "LEFT-UP"), \
                8: ButtonState(8, "NOT PRESSED")}
        buttons.append(Button([28, 29, 30, 31], [4, 5, 6 ,7], stick_push_map, "STICK PUSH"))
        
        super().__init__(buttons, "JoyCon")

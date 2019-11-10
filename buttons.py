
"""
    General class for representing a button on the Joy Con.
    Although we will holding the JoyCon on its side we will
    use naming consistent with 
    https://en-americas-support.nintendo.com/app/answers/detail/a_id/22634/~/joy-con-controller-diagram
    
    For simplicity we will refer to all buttons by their names on the RIGHT Joy Con. This is because
    both joycons are basically symmetric (and directional pad may get confusing).

    Note when refering to the direction of the directional stick we will refer to the direction to the
    top of the diagram as up.
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


# Button map for a toggleable button
toggleable_states = {1: ButtonState(1, "PRESSED"), 0: ButtonState(0, "NOT PRESSED")}

"""
    A button is defined by a list of values that map to button states, the names/outputs
    cannot repeat for an individual button and there must be a button with name "NOT PRESSED".

    Additionally a button also contains a list of bits, which correspond to the bits in the packet
    sent by the Joy Con which contains the state information. Because the Joy Con sends all the info
    for a particular button in the same byte all bits listed must be contiguous and in the same byte.

    Additionally we define bits relative to the 12 byte packet sent by the Joy Con. We define the MSB
    of the 0th byte as bit 0 and the LSB of the 11 byte as bit 95.
"""
class Button:
    def __init__(self, bit_list, button_states_map, name):
        self.byte_num, self.mask, self.shift = self._validate_bit_list (bit_list)
        self.mapping, self.state = self._validate_map (button_states_map)
        self.name = self._validate_name (name)

    """
        Takes in a list of bits, confirms that all are contiguous,
        refer to the same byte, and that the byte is in the first 11 bytes.

        Returns a tuple consisting of the byte that contains the value,
        the mask for extracting it, and the amount to shift to get the value.
    """
    def _validate_bits(self, bit_list):
        # Check that we passed in a non-zero len list
        assert(type(bit_list) == list)
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
        assert (0 <= byte_num <= 11)
        for i in byte_list[1:]:
            assert(i == byte_num)
        # Check that all bits are contiguous. We don't care about
        # the order given.
        ones_list = [7 - (bit_val % 8) for bit_val in bit_list]
        ones_list.sort(reverse=True)
        assert (len(ones_list) == (ones_list[0] - ones_list[-1]))
        # Select our shift value
        shift_value = ones_list[-1]
        # Construct our mask
        mask = 0
        for shift_val in ones_list:
            mask += (1 << bit)
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
        max_value = self.mask >> self.shift_value
        # Define the not pressed state
        not_pressed_state = None
        # Create a set for the possible states and inputs
        inputs = set()
        states = set()
        for input_val, state in states_map.items():
            assert(type(input_val) == int)
            assert(type(state) == ButtonState)
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
        return ("Button {}. It contains the following map of inputs to outputs {}.\n" \
                + "It checks its state with the mask {} on byte {}, shifting by {} bits.\n" \
                + "The button is currently in state {}") \
                .format (self.name, repr(self.mapping), hex(self.mask), \
                    self.byte_num, self.shift, self.state)

"""
    A button that only has 2 states: pressed and not pressed. It is otherwise
    the same.
"""
class ToggleButton(Button):

    def __init__(self, bit_list, name):
        super().__init__(bit_list, toggleable_states, name)


"""
    Sample class to contain information from a particular joycon
"""
class JoyCon:

    """
        To make sure each joycon is different we will create a new button
        for each joycon.
    """
    def __init__(self):
        x_button = ToggleButton([0], "x") 
        y_button = ToggleButton([0], "y")
        a_button = ToggleButton([0], "")
        b_button = ToggleButton([0], "x")

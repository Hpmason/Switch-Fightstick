import serial

from maths import clamp
from JoystickEnums import Button, Stick, HAT


class SerialManager(serial.Serial):
    debug = False

    def write_as_bytes(self, *args) -> None:
        byteArr = bytearray()
        for item in args:
            if type(item) == str:
                for char in item:
                    byteArr.append(ord(char))
            elif 0 <= item <= 255:
                byteArr.append(item)
        if self.debug:
            print(f"Writing byte array ({byteArr}) to port {self.port}")
        self.write(byteArr)

    def read_as_int_arr(self) -> tuple:
        serialBytes = self.read_all()
        intArray = []
        for singleByte in serialBytes:
            if type(singleByte) == int:
                intArray.append(singleByte)
            else:
                intArray.append(int.from_bytes(singleByte, byteorder="big"))
        return tuple(intArray)


class Payload:
    """
    Serial data payload class to handle controller serial data in order
    to prevent errors from input.
    """
    MAX_BUTTON_VALUE = 16383

    def __init__(self):
        self.left_stick = (Stick.CENTER.value, Stick.CENTER.value)
        self.right_stick = (Stick.CENTER.value, Stick.CENTER.value)
        self.hat = HAT.CENTER.value
        self.buttons = 0

    def __repr__(self):
        return f"{self.__dict__}"

    def __str__(self):
        button_list = [button.name
                       for ind, button in enumerate(Button)
                       if self.buttons & (1 << ind)]
        st = f"LeftStick: {self.left_stick}, RightStick: {self.right_stick}," + \
             f"HAT: {HAT(self.hat).name}, Buttons: {button_list}"
        return st

    def set_left_x(self, x: int) -> None:
        self.left_stick = (clamp(x, Stick.MIN.value, Stick.MAX.value), 
                        self.left_stick[1])

    def set_left_y(self, y: int) -> None:
        self.left_stick = (self.left_stick[0], 
                        clamp(y, Stick.MIN.value, Stick.MAX.value))

    def set_left_stick(self, x: int, y: int) -> None:
        self.left_stick = (clamp(x, Stick.MIN.value, Stick.MAX.value),
                            clamp(y, Stick.MIN.value, Stick.MAX.value))

    def set_right_x(self, x: int) -> None:
        self.right_stick = (clamp(x, Stick.MIN.value, Stick.MAX.value),
                        self.right_stick[1])

    def set_right_y(self, y: int) -> None:
        self.right_stick = (self.right_stick[0], clamp(y, Stick.MIN.value, Stick.MAX.value))

    def set_right_stick(self, x: int, y: int) -> None:
        self.right_stick = (clamp(x, Stick.MIN.value, Stick.MAX.value),
                           clamp(y, Stick.MIN.value, Stick.MAX.value))

    def set_hat(self, x, y) -> None:
        dpad_list = [
            [7, 0, 1],
            [6, 8, 2],
            [5, 4, 3]
        ]
        self.hat = dpad_list[y + 1][x + 1]

    def apply_buttons(self, *args: Button) -> None:
        if not len(args) > 0:
            return
        for item in args:
            if type(item) == Button:
                value = item.value
            else:
                value = item
            self.buttons |= value
        self.buttons = clamp(self.buttons, 0, self.MAX_BUTTON_VALUE)

    def release_all_buttons(self) -> None:
        self.buttons = 0

    def reset_inputs(self) -> None:
        self.left_stick = (Stick.CENTER.value, Stick.CENTER.value)
        self.right_stick = (Stick.CENTER.value, Stick.CENTER.value)
        self.buttons = 0
        self.hat = HAT.CENTER.value

    def as_byte_arr(self) -> bytearray:
        buttons1, buttons2 = (b for b in self.buttons.to_bytes(2, byteorder="little"))
        return bytearray([self.left_stick[0], self.left_stick[1],
                          self.right_stick[0], self.right_stick[1],
                          self.hat, buttons1, buttons2])

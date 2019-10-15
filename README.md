# NOTE: Currently doesn't work on switch, will be updating soon
I've been having some issues sending data to the microcontroller while connected to the switch. It works fine while connected to PC/Mac. Will be looking into this and fixing it as soon as I can.

## Switch UART Controller Tools
UART Reference/Library for Switch-FightStick Controller

Uses the LUFA library and reverse-engineering of the Pokken Tournament Pro Pad for the Wii U to enable custom fightsticks on the Switch System v3.0.0

Required libaries for helper classes: `pyserial`

Required libraries for example.py: `pygame`

Components used in this project: `Teensy 2.0++`, `USB to UART bridge`

Inspired by (and forked from) ShinyQuagsire's Splat printer (https://github.com/shinyquagsire23/Switch-Fightstick) and PiManRules' Super Mario Odyssey bots (https://www.youtube.com/watch?v=hu3HEwc6Pwk&list=PLRqz09NxzVqYLX_1F3xB01hgpDFpSxUEi)

## Writing Your Own Automated Programs
This project is meant for a starting point on writing bots for the the switch. The main branch example makes use of pygame in order to get the user's keyboard and mouse input and modify the serial data according to how the buttons are mapped. You can modify the csv file "controllerMapping.csv" and use the accepted keys (taken from pygames list of key names) found in the "keys.txt" file.

Feel free to fork this repo as a basis your own projects and if you want to, help contribute to make this one better for new users looking for a place to get started.

This is my first project trying to make classes that can be used for such a wide range of projects, so I apologize for some of the classes being a bit messy.

#### Compiling C and Flashing onto the Teensy 2.0++
Go to the Teensy website and download/install the [Teensy Loader application](https://www.pjrc.com/teensy/loader.html). For Linux, follow their instructions for installing the [GCC Compiler and Tools](https://www.pjrc.com/teensy/gcc.html). For Windows, you will need the [latest AVR toolchain](http://www.atmel.com/tools/atmelavrtoolchainforwindows.aspx) from the Atmel site. See [this issue](https://github.com/LightningStalker/Splatmeme-Printer/issues/10) and [this thread](http://gbatemp.net/threads/how-to-use-shinyquagsires-splatoon-2-post-printer.479497/) on GBAtemp for more information. (Note for Mac users - the AVR MacPack is now called AVR CrossPack. If that does not work, you can try installing `avr-gcc` with `brew`.)

Next, you need to grab the LUFA library. You can download it in a zipped folder at the bottom of [this page](http://www.fourwalledcubicle.com/LUFA.php). Unzip the folder, rename it `LUFA`, and place it where you like. Then, download or clone the contents of this repository onto your computer. Next, you'll need to make sure the `LUFA_PATH` inside of the `makefile` points to the `LUFA` subdirectory inside your `LUFA` directory. My `Switch-Fightstick` directory is in the same directory as my `LUFA` directory, so I set `LUFA_PATH = ../LUFA/LUFA`.

Now you should be ready to rock. Open a terminal window in the `Switch-Fightstick` directory, type `make`, and hit enter to compile. If all goes well, the printout in the terminal will let you know it finished the build! Follow the directions on flashing `Joystick.hex` onto your Teensy, which can be found page where you downloaded the Teensy Loader application.

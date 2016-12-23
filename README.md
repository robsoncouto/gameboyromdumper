# gbcart tool (in progress)
Tool for reading data from Game Boy cartridges or the Gameboy camera. The version in the first commits work(mar 2016), but I have started to clean the code, so the present version (dec 2016) is uncomplete. 
##Description
This is a project that I started in order to backup my Gameboy saves to the PC. It also dumps roms and Gameboy camera saves. All the process that lead to this program is detailed at www.dragaosemchama.com.br/?lang=en_us
##Hardware
Schematic and layout ealge files are available, the cartridge-PC interface board uses a atmega32 and a Gameboy Color cartridge socket. The socket was salvaged from a broken gameboy. The board is connected to a PC with a USB-serial ttl cable.
##Usage
The python script is used for reading saves, roms or the cartridge header. The microcontroller code was written for a Atmega32, but can be easily ported to other microcontrollers or architectures.
##Serial library
This code uses the serial library kindly shared by Peter Fleury
http://homepage.hispeed.ch/peterfleury/avr-software.html

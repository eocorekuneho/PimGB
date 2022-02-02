
# PimGB
Send MIDI data to Game Boy

This is a small, draft like, heavily WIP project to bring the functionalities of the Arduinoboy (https://github.com/trash80/arduinoboy) to the Raspberry Pi Zero.

Currently it's only useful with mGB (https://github.com/defensem3ch/mGB)

You can play the channels one-by-one, and change amongst them sending Program Change 0-5 on Channel 10 to the software.

My far goal with this project is to create a small, convenient and smart USB MIDI host out from the Raspberry Pi Zero. One of its modules is this project, PimGB as a "driver" for Game Boys.

### Wiring:
|Game Boy Link cable PIN#|Raspberry Pi PIN# (wiringPi)  |
|--|--|
| 2 (SOUT) (not in use) | 6 |
| 3 (SIN) | 5 |
| 5 (SCLK) | 4 |
But all of the parameters are configurable.

### Usage
Compile it with gcc:

    gcc -lwiringPi -o main main.c

Run it:

    sudo ./main

The software reads from /dev/midi1 (at the time) and bitbangs the data to the Game Boy.


THIS IS REALLY REALLY WIP.
A snapshot of the code.

Also, the code may be completely garbage. This is my first C code in 10 years, haven't touched it since then.

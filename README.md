# DaltonAJKI

## Description

AJKI means Dalton's Atari Joystick and Keyboard Interface

It's a piece of software for Arduino Uno that makes use of MAX3421E based USB Host Shield board and USB Host Shield library. It is an interface to connect an USB joystick, gamepad or keyboard to 8-bit Atari computer.

There is already the project called AKI: http://ftp.pigwa.net/stuff/mirror/www.atari.cuprum.com.pl/aki.htm
It allows to connect PS2 keyboard to Atari. The idea of DaltonAJKI was to create a modern replacement, because PS2 keyboards are kind of obsolete nowadays. Moreover, I wanted to utilize more than one gamepad button in order to handle "Start", "Select" and "Option" from the gamepad, not from the keyboard.

The initial version (2019-IV-30) can only handle PlayStation 4 gamepad connected by cable. Support for other peripherals is planned.

## Remarks

Use it at your own risk. Remember that there are two versions of USB Host Shield: 3.3V and 5V. Atari works with 5V. This code was tested with 5V-version of USB Host Shield only and won't work with 3.3V.

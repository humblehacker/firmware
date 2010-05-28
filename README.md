Matrix Discovery Mode
=====================

This is a special firmware mode that can help you to determine the 
keyboard matrix for a keyboard that is not yet supported by the 
HumbleHacker firmware.  Basically, you wire all the leads on your 
keyboards connector to pins on your controller board, then build and 
install the specially modified firmware.  Once the firmware is loaded,
the keyboard will interactively walk you through discovering the matrix.

Usage
-----

- Edit the makefile, and uncomment the line

    `MATRIX_MODE = 1`

- Edit the file 'matrix_discovery_defs.h' to specify which ports/pins are 
  connected to your keyboard matrix.  For example, if you connected one of 
  the leads on your keyboard matrix to port D pin 3, you would find and 
  uncomment the line:

    `#define USE_PIND3`

- Build the firmware, and program it into your controller board.  

- When the firmware resets, it will pause for a few seconds.  During this 
  time, switch to a text-editor such as TextEdit or Notepad.  Make sure
  your keymap is set to US QWERTY. After the pause, the firmware will 
  begin to write instructions to the editor.

- Follow the instructions to discover your matrix!

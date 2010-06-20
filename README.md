Getting Started
===============


Things you'll need
------------------
* an Atmel AT90USB based controller board
* avr-gcc 4
* ruby 1.8.7
* HumbleHacker firmware source code


### Controller board

You can use Atmel's [AT90USBKey][usbkey] demo board, but a better choice may be PJRC's [Teensy++][teensy].

[usbkey]:http://www.atmel.com/dyn/products/tools_card.asp?tool_id=3879
[teensy]:http://www.pjrc.com/teensy/

Alternately, you can design and build your own AT90USB-based controller board.  Any of AT901286/7 or AT90646/7 MCUs will work.


### AVR-GCC

There are avr-gcc packages for the big three platforms.  Installation instructions can be found on each site.

- Windows: [WinAVR][winavr]
- Mac OSX: [AVR CrossPack][crosspack]
- Linux:   depends on the distribution.  For example, on Ubuntu execute: `sudo apt-get install gcc-avr avr-libc`

[winavr]:http://winavr.sourceforge.net/
[crosspack]:http://www.obdev.at/products/crosspack/index.html


### Ruby 1.8.7

Again, there are different packages for different platforms.

- Windows: [RubyInstaller][rubyinst].  NOTE: When installing, choose the option to "Add ruby executables to your PATH" (The file association option is up to you).
- Mac OSX: Ruby 1.8.7 is installed by default with Snow Leopard (10.6).  If you have an earlier version of OS X, you can install Ruby via [MacPorts][macports]

Once Ruby is installed, there one additional dependency that needs to be taken care of.  Execute the following command to install [treetop].

    gem install treetop

[rubyinst]:http://rubyforge.org/frs/download.php/71067/rubyinstaller-1.8.7-p249.exe
[macports]:http://www.macports.org/
[treetop]:http://treetop.rubyforge.org/

### HumbleHacker source code

Although there are [tarballs and zipfiles][hh_downloads] available, it's best to just install and use [git].  That way you can easily update the source when bugs are fixed and features added.  [GitHub] has good [git installation instructions][git_install] for each platform.

The source code can be found [here][hh_source].  To get it, from the command-line, execute:

    git clone git://github.com/humblehacker/keyboard.git
        
This will place the source code in a directory `keyboard` in your current directory.

[hh_downloads]:http://github.com/humblehacker/keyboard/downloads
[hh_source]:http://github.com/humblehacker/keyboard
[github]:http://github.com
[git_install]:http://help.github.com/git-installation-redirect
[git]:http://git-scm.com/

Building the firmware
=====================

Now that everything is in place, you can build and install the firmware.  Broadly speaking, we'll be performing the following steps:

- create your keyboard/controller specific project directory
- test build
- modify sources and makefile for your keyboard/controller
- test build and program
- modify layout files as desired

Instantiate project directory
-----------------------------
Navigate to `keyboard/firmware`.  In this directory, you'll find a directory `kb_reference`.  This is your starting point for building a firmware specific to your keyboard/controller combo.  You need to duplicate this directory, naming it whatever you desire.  It's a good idea to keep the `kb_` prefix. In this example, I'll use `kb_example`.

Test build
----------
To ensure that your build environment is set up correctly, and all dependencies have been satisfied, it's a good idea to try to build the firmware prior to making any code modifications.  From the command-line, navigate to `keyboard/firmware/kb_example` and type `make`.  At this point you should have a successful build.  If not, review the above instructions to make sure you haven't missed anything.  If you're still having problems, contact me and we'll work them out.

Modify sources
--------------
There are a few things you'll need to change before you can program your controller with this firmware.  All of the following files/paths are relative to the `kb_example` directory.

- makefile
- layouts/Matrix.kspec
- Board/LEDs.h
- Board/Identifiers.h

### makefile

You need to know two things before modifying the makefile: which microcontroller your controller will be using, and what its clock speed is.

With this information in hand, edit the makefile, and look for the line that reads: `MCU = at90usb1287` (roughly line 66).  Change `at90usb1287` to whatever MCU is used on your controller board.  If you're using an Atmel AT90USBKey, nothing needs to change.  If you're using a Teensy++, change it to read `MCU = at90usb1286`.

Next, look for the line that reads:

    F_CPU = 8000000
    
If your clock speed is 8Mhz, nothing needs to change (USBKey).  Otherwise, if your clock speed is 16Mhz, change it to read:

    F_CPU = 16000000

### layouts/Matrix.kspec

Here is where you define your keyboard matrix, and how that matrix is wired up to your controller board.  Each key on your keyboard needs a unique identifier.  A very simple method is to use numbers for rows and letters for columns.  For example, on a standard QWERTY keyboard with the ESC key in the upper left, that key would have the identifier `1A` (1st row, 1st col).  Similarly, the Q key would have the identifier `3B` (3rd row, 2nd col).  If you already know your matrix, all you have to do modify the Matrix table in layouts/Matrix.kspec.  For example, if the Q key (location `1A`) is at row 0, column 12 of the matrix, you would modify the first line of the matrix definition as follows:

    Matrix:Reference
    /*            0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17 */
    /* 0 */ Row: --, --, --, --, --, --, --, --, --, --, --, 1A, --, --, --, --, --, --

### Board/LEDs.h

Coming soon.

### Board/Identifiers.h

Coming soon.

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

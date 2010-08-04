Getting Started
===============


Things you'll need
------------------
* an Atmel AT90USB based controller board
* avr-gcc 4
* kspec code generator
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

### kspec code generator

There are pre-built versions for each of the major platforms.  Go to the [downloads] page on github and grab the latest version for your platform.  Once you've downloaded it, extract the archive and place the resulting binary in a directory in your path.  `/usr/local/bin` is a good choice.

[downloads]:http://github.com/humblehacker/keyboard/downloads


### HumbleHacker source code

Although there are [tarballs and zipfiles][hh_downloads] available, it's best to just install and use [git].  That way you can easily update the source when bugs are fixed and features added.  [GitHub] has good [git installation instructions][git_install] for each platform.

The source code can be found [here][hh_source].  To get it, from the command-line, execute:

    git clone git://github.com/humblehacker/keyboard.git
        
This will place the source code in a directory called `keyboard` in your current directory.

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
- config.kspec
- Board/LEDs.h
- Board/Identifiers.h

### makefile

You need to know two things before modifying the makefile: which microcontroller your controller will be using, and what its clock speed is.

With this information in hand, edit the makefile, and look for the line that reads: `MCU = at90usb1287` (roughly line 66).  Change `at90usb1287` to whatever MCU is used on your controller board.  If you're using an Atmel AT90USBKey, nothing needs to change.  If you're using a Teensy++, change it to read `MCU = at90usb1286`.

Next, look for the line that reads:

    F_CPU = 8000000
    
If your clock speed is 8Mhz, nothing needs to change (USBKey).  Otherwise, if your clock speed is 16Mhz, change it to read:

    F_CPU = 16000000

### config.kspec

`config.kspec` is where all the fun happens.  In this file, you define your keyboard matrix, how that matrix is connected to your controller board, and all of the various keymaps you wish to define for your keyboard.  The default file comes preloaded with useful maps, and with a bit of modification, you should be up and typing!

Fire up your favorite text editor, and open `config.kspec`.  Scroll down past the GPL commentary until you reach the line:

    Keyboard:HumbleHackerReference

Change `HumbleHackerReference` to an identifier of your choice.  For example, if you're modding a Kinesis Contoured keyboard, you would change it to something like:

    Keyboard:KinesisContoured

Now let's go through the different sections, starting with the keyboard matrix.

#### Matrix

Move down the file a couple of lines until you come to the line:

    Matrix:

Here is where you define your keyboard matrix, and how that matrix is wired up to your controller board.  Each key on your keyboard needs a unique identifier.  A very simple method is to use numbers for rows and letters for columns.  For example, on a standard QWERTY keyboard with the ESC key in the upper left, that key would have the identifier `1A` (1st row, 1st col).  Similarly, the Q key would have the identifier `3B` (3rd row, 2nd col).  If you already know your matrix, all you have to do fill in this table.  For example, if the Q key (location `1A`) is at row 0, column 12 of the keyboard matrix, you would modify the first line of the matrix definition as follows:

    Matrix:
    /*            0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 */
    /* 0 */ Row: -- -- -- -- -- -- -- -- -- -- -- 1A -- -- -- -- -- --
    /* 1 */ Row: ...

But you're not limited to this scheme.  If you prefer to just use the labels on the keys, you can do that too, with a few exceptions.  Using the previous example, with the Q key at matrix row 0 column 12, your matrix definition would look like this:

    Matrix:
    /*            0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 */
    /* 0 */ Row: -- -- -- -- -- -- -- -- -- -- --  Q -- -- -- -- -- --
    /* 1 */ Row: ...

The only problem with this is that your identifiers can't be special characters, (e.g. `\`, `<`, and `|`), so you'll have to come up with a good mnemonic.

**If you don't know your matrix, see the section _Matrix Discovery Mode_, below.**

After you define the matrix, you'll also have to define how the rows and columns of the matrix are connected to the controller board, or more specifically, which pins on the controlller's MCU are connected to which rows and columns of the matrix.  Scroll down a few lines until your come to the `ColPins:` and `RowPins:` definitions.

Let's say your keyboard has a matrix of 16 columns and 8 rows.  Your `ColPins:` entry would then be followed by 16 _pin identifers_ and `RowPins:` would be follewed by 8 _pin identifiers_.  What's a _pin identifier_?  Your MCU has various _I/O ports_ (identified by letter), and each of these ports is divided into a set of _pins_ (identified by number).   For example, the Atmel AT90USB1286/7 (MCU on the Teensy++ and the USBKey) has 6 ports, `PORTA` - `PORTF`.  On each of these ports there are 8 pins, labeled 0 - 7.  So the first pin on `PORTC` is labeled `PC0`.  `PC0` is its _pin identifier_.   

Once you've determined the _pin identifier_ for each matrix row and column, just fill in the `ColPins` and `RowPins` entries.  The following is an actual example from the Humble Hacker I keyboard:

    /*       0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15 */
    ColPins:PC0 PC1 PC2 PC3 PC4 PC5 PC6 PC7 PD7 PD6 PD5 PD4 PD3 PD2 PD1 PD0
  
    /*       0   1   2   3   4   5   6  */
    RowPins:PA0 PA1 PA2 PA3 PA4 PA5 PA6

The final item in the matrix section is a simple on/off switch.

    BlockGhostKeys: yes or no

For a good explanation of keyboard ghosting, see Dave Dribin's [Keyboard Matrix Help] page.  Basically, if you are modding a membrane keyboard, you want to turn this on:

    BlockGhostKeys:yes

Otherwise, if your keyboard has mechanical keyswitches, most likely you want this off:

    BlockGhostKeys:no

[Keyboard Matrix Help]:http://www.dribin.org/dave/keyboard/one_html/


<span style="color:red">**NOTE: the following process has changed and will soon be updated**</span>

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


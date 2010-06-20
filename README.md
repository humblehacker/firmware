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

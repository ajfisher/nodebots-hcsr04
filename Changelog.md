# Changelog for HC-SR04 ultrasonic sensor

## 0.5.0

* Updated to firmata 2.5.1
* Merged in preferred methods of doing cability mapping on the pins to ensure
they are allowed to do pin reads (which firmata.js looks for to see if it can use
this controller)

### 0.4.1

* Updated to use `pro-mini` board type to be in line with avrgirl.
* Better docs
* updated all the bins

## 0.4.0

* Made interchange more literal rather than trying to deal with bytes encoded
in weird ways. Will now just read a number as a sequence of chars.

### 0.3.1

* Fixed dev dependency requirements.

## 0.3.0

* Modified build script to use dynamic tasks for the execution of the compilation step
* Ported original gist file of I2C backpack across and updated
* Updated grunt for the backpack build process.
* First draft of interchange library which can be used for other projects.

### 0.2.2

* updated manifest file to reflect new structure to account for both custom firmata
and backpack capable firmware.
* Updated tests and manifest to reflect the way arduino is producing the hex files in
1.6.6 Nightly
* Fixed promini in line with avrgirl

### 0.2.0

* Added start of test framework including test to ensure that the hex files all
exist and are non zero length and use of manifest to guide the test process.
* Changes to manifest log in order to provide better
* refactor of build to use `pro-mini` naming per @noopkat's avrgirl.

### 0.1.1

* refinement of some grunt tasks
* refinement of build process because arduino nightly has changed the build folder
output
* preparation for upstream integration with avrgirl
* adding of manifest files to support the point above.

## 0.1.0

* Initial set up
* grunt tasks for build of firmware structure and cleaning directories
* Initial documentation
* Demo firmware using firmata to show build process.
* set up environment variable script.


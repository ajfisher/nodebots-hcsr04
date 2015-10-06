# Changelog for HC-SR04 ultrasonic sensor 

### 0.2.1

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


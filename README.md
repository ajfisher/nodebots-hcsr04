# HC-SR04 Ultrasonic Sensor johny-five connector

By default HCSR04 ultrasonic sensors don't work with johnny-five. This is because
they use a custom protocol that requires pulseIn which is not part of the core
firmata. Similarly for devices like Raspberry Pis, you can't directly connect
to these devices. 

As such a "BackPack" provides the ability to use devices such as this using 
I2C which is supported by all Johnny-Five boards. More info on this can be
found at [NodeBots Interchange](http://github.com/ajfisher/nodebots-interchange).

## Package Installation

Installation can be done manually using:

```
npm install nodebots-hcsr04
```

However if you are using johnny five then you can use 
[Interchange](http://github.com/ajfisher/nodebots-interchange) instead

## Firmware installation using Interchange

Interchange is the preferred method of installation with the following instruction
being the way to do it.

```
interchange install hc-sr04 [--firmata] [board args]
```

Using the `--firmata` switch will install the custom firmata with  

Currently supported devices are:

* Arduino Nano (preferred target)
* Arduino ProMini
* Arduino Uno

A "supported board" means that precompiled binaries are shipped in the repo. This
means you don't need to install arduino to be able to install this on your board 
(this is especially useful if you're on a chromebook). All you need to do is:

```
interchange install hcsr04 -p <port> -b <board>
```

If your board isn't supported then it's still pretty easy but you will need
arduino or GCC. Go to the `firmware/build/hcsr04_backpack` folder and compile
the hcsr04_backpack.ino file and then upload it to the board using whatever
usual processes you have.

## Developing

Generally speaking it's a case of simply cloning the repo, doing an npm install
and doing development as normal.

One thing to note is that all pull requests require rebuilding the application 
in order to generate a new set of builds. For this it's handy to make sure you
have the `ARDUINO_PATH` environment variable set and then a `grunt build` is
all you need.

`ARDUINO_PATH` is simply a full path to the arduino run time, noting that you also
must have a version more recent than 1.6.2 in order to get command line benefits.

To make a build, simply `grunt compile` and it should do everything needed.

### Arduino Version

Currently this repo uses the Arduino 1.6.6 build in order to maintain consistency
of build process with regards to where compiled Hexes end up.

## Tests

Before issuing a pull request, please deliver appropriate tests and also make
sure the test suite has been run completely - this will ensure build and 
hex files are in the right location.

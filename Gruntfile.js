var arduino = process.env.ARDUINO_PATH;

var boards = {
    "uno" :{
        package: "arduino:avr:uno",
    },
    "nano": {
        cpu: "atmega328",
        package: "arduino:avr:nano:cpu=atmega328",
    },
    "pro-mini": {
        cpu: "16MHzatmega328",
        package: "arduino:avr:pro:cpu=16MHzatmega328",
    },
};

var boardlist = Object.keys(boards).toString();

module.exports = function(grunt) {

    // configure the tasks
    grunt.initConfig({
        // exec: create task dynamically see end of file for where this happens.
        copy: {
            options: {
                timestamp: true,
            },

            firmata: {
                cwd: 'firmware/src/',
                flatten: true,
                src: [ 'libs/**', 'controller_src/hcsr04_firmata/*' ],
                dest: 'firmware/build/hcsr04_firmata/',
                expand: true,
                filter: 'isFile',
            },
            backpack: {
                cwd: 'firmware/src/',
                flatten: true,
                src: [ 'controller_src/hcsr04_backpack/*' ],
                dest: 'firmware/build/hcsr04_backpack/',
                expand: true,
                filter: 'isFile',
            },
        },
        clean: {
            firmware_build: {
                src: [
                        'firmware/build/hcsr04_firmata',
                        'firmware/build/hcsr04_backpack',
                     ]
            },
            compiled_bins: {
                src: [
                        'firmware/bin/backpack/*',
                        'firmware/bin/firmata/*'
                    ]
            },
            post_compile: {
                src: [
                        'firmware/bin/backpack/{' + boardlist + '}/!(*ino.hex)',
                        'firmware/bin/firmata/{' + boardlist + '}/!(*ino.hex)'
                    ]
            },
        },
        nodeunit: {
            all: ['test/',],
            options: {
                reporter: "verbose",
            },
        },
    });

    // load the tasks
    grunt.loadNpmTasks('grunt-contrib-copy');
    grunt.loadNpmTasks('grunt-contrib-clean');
    grunt.loadNpmTasks('grunt-exec');
    grunt.loadNpmTasks('grunt-contrib-nodeunit');

    // dynamically create the compile targets for the various boards
    Object.keys(boards).forEach(function(key) {
        grunt.config(["exec", "firmata_" + key], {
            command:function() {
                return arduino + " --verify --verbose-build --board "  + boards[key].package +
                " --pref build.path=firmware/bin/firmata/" + key +  " firmware/build/hcsr04_firmata/hcsr04_firmata.ino";
            },
        });
        grunt.config(["exec", "backpack_" + key], {
            command:function() {
                return arduino + " --verify --verbose-build --board "  + boards[key].package +
                " --pref build.path=firmware/bin/backpack/" + key +  " firmware/build/hcsr04_backpack/hcsr04_backpack.ino";
            },
        });
    });

    grunt.registerTask('test', ['nodeunit:all']);
    grunt.registerTask('build', ['clean:firmware_build', 'clean:compiled_bins', 'copy']);
    grunt.registerTask('compile', ['build', 'exec', 'clean:post_compile']);
};

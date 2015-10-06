var fs = require("fs");
var path = require("path");
var manifest = require("../manifest.json");

var boards = ["uno", "nano", "promini"];

exports["Devices - Firmata Library"] = {
    setUp: function(done) {
        done();
    },

    tearDown: function(done) {
        done();
    },

    firmataFirmwareExists: function(test) {
        // tests that the builds exist properly.
        test.expect(boards.length);
        boards.forEach(function(board) {
            var p = path.join(path.dirname(__filename), "..", manifest.firmata.bins, board , manifest.firmata.hexPath);
            try {
                // test file exists
                var file_stat = fs.statSync(p);
                test.ok(true, board + " hex file exists");
            } catch (e) {
                test.ok(false, board + " hex file doesn't exist at " + p);
            }
        });
        test.done();
    },

    firmataFirmwareNonZero: function(test) {
        test.expect(boards.length);
        boards.forEach(function(board) {
            var p = path.join(path.dirname(__filename), "..", manifest.firmata.bins, board , manifest.firmata.hexPath);
            // test file has bytes
            var file_stat = fs.statSync(p);
            test.notEqual(file_stat.size, 0, board + " hex file is not Zero bytes");
        });
        test.done();
    },
};


//return arduino + " --verify --verbose-build --board "  + boards[board].package +
//                    " --pref build.path=bin/firmata/" + board +  " build/hcsr04_firmata/hcsr04_firmata.ino";

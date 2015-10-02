var fs = require("fs");
var path = require("path");
var manifest = require("../manifest.json");

var boards = ["uno", "nano"];

exports["Devices - Library"] = {
    setUp: function(done) {
        done();
    },

    tearDown: function(done) {
        done();
    },

    firmwareExists: function(test) {
        // tests that the builds exist properly.
        test.expect(boards.length);
        boards.forEach(function(board) {
            var p = path.join(path.dirname(__filename), "..", manifest.bins, board , manifest.hexPath);
            try {
                // test file exists
                var file_stat = fs.statSync(p);
                test.ok(true, "File exists");
            } catch (e) {
                test.ok(false, "Hex file " + p + " doesn't exist");
            }
        });
        test.done();
    },

    firmwareNonZero: function(test) {
        test.expect(boards.length);
        boards.forEach(function(board) {
            var p = path.join(path.dirname(__filename), "..", manifest.bins, board , manifest.hexPath);
            // test file has bytes
            var file_stat = fs.statSync(p);
            test.notEqual(file_stat.size, 0, "Hex file " + p + " is Zero bytes");
        });
        test.done();
    },
};


//return arduino + " --verify --verbose-build --board "  + boards[board].package +
//                    " --pref build.path=bin/firmata/" + board +  " build/hcsr04_firmata/hcsr04_firmata.ino";

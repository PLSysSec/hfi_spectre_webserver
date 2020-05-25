'use strict'

const autocannon = require('../node_modules/autocannon')
const fs = require('fs')
const util = require('util')
const fs_writeFile = util.promisify(fs.writeFile)


function sleep(ms) {
    return new Promise((resolve) => {
        setTimeout(resolve, ms);
    });
}

function runAutoCannon(name, path, expected) {
    console.log("Running " + name);
    var options = {
        title: name,
        url: 'http://localhost:8000' + path
    };
    if (expected) {
        options["expectBody"] = expected;
    }
    var r = autocannon(options);
    autocannon.track(r, { renderLatencyTable: true })
    return r;
}

async function runTestWithProtection(path, expected) {
    await runAutoCannon("Warmup " + path, path, expected)
    await sleep(5000);
    return await runAutoCannon("Testing " + path, path, null);
}

async function runTest(protections, test) {
    var results = {};
    for (var i = 0; i < protections.length; i++) {
        var p = protections[i];
        var inputString = test.inputs;
        if (inputString) {
            inputString += "&";
        }
        var path = "/" + test.module + "?" + inputString + "protection=" + p;
        results[p] = await runTestWithProtection(path, test.expected)
    }
    return results;
}

async function runTests(protections, tests) {
    var results = {};
    for (var i = 0; i < tests.length; i++) {
        var test = tests[i];
        var r = await runTest(protections, test);
        results[test] = r;
    }
    return results;
}

async function main() {
    var myArgs = process.argv.slice(2);
    if (myArgs.length != 1) {
        console.log("Expected one arg [--cet|--nocet]");
        process.exit(1);
    }

    var protections;

    if (myArgs[0] == "--cet") {
        protections = ["spectre_cet", "spectre_cet_no_cross_sbx"];
    } else if (myArgs[0] == "--nocet") {
        protections = ["stock", "spectre_sfi", "spectre_sfi_no_cross_sbx"];
    } else {
        console.log("Expected one arg [--cet|--nocet]");
        process.exit(1);
    }

    var results = await runTests(protections, [
        {
            module: "fib_c",
            inputs: "num=4",
            expected: "3\n"
        },
        {
            module: "msghash_check_c",
            inputs: "msg=hello&hash=2CF24DBA5FB0A30E26E83B2AC5B9E29E1B161E5C1FA7425E73043362938B9824",
            expected: "Succeeded\n"
        },
        {
            module: "jpeg_resize_c",
            inputs: "quality=20",
        },
        {
            module: "html_template",
            inputs: "",
        },
    ]);
    var filename = "results.json";
    console.log("Writing results of benchmarking to file: " + filename);
    await fs_writeFile(filename, JSON.stringify(results));
}

main();
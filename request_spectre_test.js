'use strict'

const autocannon = require('../node_modules/autocannon')
const fs = require('fs')
const util = require('util')
const fs_writeFile = util.promisify(fs.writeFile)
const fs_readFile = util.promisify(fs.readFile);

function sleep(ms) {
    return new Promise((resolve) => {
        setTimeout(resolve, ms);
    });
}

function runAutoCannon(name, path, body, expected, duration) {
    console.log("Running " + name);
    var options = {
        title: name,
        url: 'http://localhost:8000' + path
    };
    if (body) {
        options["body"] = body;
        options["method"] = "POST";
    }
    if (expected) {
        options["expectBody"] = expected;
    }
    if (duration) {
        options["duration"] = duration;
    }
    var r = autocannon(options);
    autocannon.track(r, { renderLatencyTable: true })
    return r;
}

async function runTestWithProtection(path, body, expected, duration) {
    await runAutoCannon("Warmup " + path, path, body, expected, 10);
    await sleep(5000);
    return await runAutoCannon("Testing " + path, path, body, null, duration);
}

async function runTests(protections, tests) {

    var results = {};
    for (var i = 0; i < protections.length; i++) {
        var p = protections[i];
        results[p] = {};
        for (var j = 0; j < tests.length; j++) {
            var test = tests[j];
            var inputString = test.inputs;
            if (inputString) {
                inputString += "&";
            }
            var path = "/" + test.module + "?" + inputString + "protection=" + p;
            results[p][test.module] = await runTestWithProtection(path, test.body, test.expected, test.duration);
        }
    }
    return results;
}

async function main() {
    var myArgs = process.argv.slice(2);
    if (myArgs.length != 1) {
        console.log("Expected one arg [--cet|--nocet]");
        process.exit(1);
    }

    console.log("Make sure to restart the webserver on each run. Tests with core protections mess up the affinities of server threads.");
    await sleep(5000);

    var protections;

    if (myArgs[0] == "--cet") {
        protections = ["spectre_cet_aslr", "spectre_cet_full"];
    } else if (myArgs[0] == "--nocet") {
        protections = ["stock", "spectre_sfi_aslr", "spectre_sfi_full"];
    } else {
        console.log("Expected one arg [--cet|--nocet]");
        process.exit(1);
    }

    var xmlContents = await fs_readFile("./request_spectre_test.xml");
    var xmlContentsStr = encodeURI(xmlContents);

    var results = await runTests(protections, [
        // {
        //     module: "fib_c",
        //     inputs: "num=4",
        //     expected: "3\n"
        // },
        {
            module: "msghash_check_c",
            inputs: "msg=hello&hash=2CF24DBA5FB0A30E26E83B2AC5B9E29E1B161E5C1FA7425E73043362938B9824",
            expected: "Succeeded\n",
            duration: 180
        },
        {
            module: "html_template",
            inputs: "",
            duration: 180
        },
        {
            module: "xml_to_json",
            inputs: "",
            body: xmlContentsStr,
            duration: 180
        },
        {
            module: "jpeg_resize_c",
            inputs: "quality=20",
            duration: 180
        },
        {
            module: "tflite",
            inputs: "",
            duration: 180
        },
    ]);
    var filename = "results.json";
    console.log("Writing results of benchmarking to file: " + filename);
    await fs_writeFile(filename, JSON.stringify(results));
}

main();

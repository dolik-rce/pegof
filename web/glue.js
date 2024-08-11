let input = document.getElementById("input");
let output = document.getElementById("output");
let logs = document.getElementById("log");
let load = document.getElementById("load");
let runBtn = document.getElementById("btnRun");
let auto = document.getElementById("u");
let peg = null;
let loaded = false;
let verbose = false;

function log(...args) {
    if (verbose) {
        console.log(...args);
    }
}

function stats(path, title) {
    window.goatcounter.count({
        path:  path,
        title: title,
        event: true
    });
}

function compress(c){var x="charCodeAt",b,e={},f=c.split(""),d=[],a=f[0],g=256;for(b=1;b<f.length;b++)c=f[b],null!=e[a+c]?a+=c:(d.push(1<a.length?e[a]:a[x](0)),e[a+c]=g,g++,a=c);d.push(1<a.length?e[a]:a[x](0));for(b=0;b<d.length;b++)d[b]=String.fromCharCode(d[b]);return d.join("")}

function decompress(b){var a,e={},d=b.split(""),c=f=d[0],g=[c],h=o=256;for(b=1;b<d.length;b++)a=d[b].charCodeAt(0),a=h>a?d[b]:e[a]?e[a]:f+c,g.push(a),c=a.charAt(0),e[o]=f+c,o++,f=a;return g.join("")}

function base64Decode(base64) {
    const binString = atob(base64);
    return new TextDecoder().decode(Uint8Array.from(binString, (m) => m.codePointAt(0)));
}

function base64Encode(str) {
    const binString = Array.from(new TextEncoder().encode(str), (byte) => String.fromCodePoint(byte)).join("");
    return btoa(binString);
}

function serializeState() {
    let state = { o: [], v: {} };
    if (input.value != peg) {
        state.i = input.value;
    } else {
        state.p = load.selectedOptions[0].id;
    }
    document.querySelectorAll("input").forEach(x => {
        let v = (x.type == "checkbox") ? (x.checked ? 1 : 0) : x.value;
        if (x.dataset.default != v) {
            state.v[x.id] = v;
        }
    });
    document.querySelectorAll("option:checked").forEach(x => {
        if (x.dataset.default == undefined) {
            state.o.push(x.id);
        }
    });
    log("Serialized", state);
    history.replaceState(null,null,"#" + base64Encode(compress(JSON.stringify(state))));
}

function deserializeState() {
    let raw = location.hash.substr(1);
    if (!raw) {
        loadPeg();
        return;
    }
    let state = JSON.parse(decompress(base64Decode(raw)));
    log("Deserialized", state);

    if (state.i) {
        input.innerHTML = state.i;
        peg = null;
    } else {
        document.getElementById(state.p).selected = true;
        loadPeg();
    }
    state.o.forEach(opt => document.getElementById(opt).selected = true);
    Object.entries(state.v).forEach(([k,v]) => {
        let x = document.getElementById(k);
        if (x.type == "checkbox") {
            x.checked = v == 1;
        } else {
            x.value = v;
        }
    });
}

function launch() {
    FS.writeFile("/tmp/input.peg", input.value);
    output.innerHTML = "";
    logs.innerHTML = "";

    let options = [];
    document.querySelectorAll(".opt option:checked").forEach(x => options.push(...x.value.split(" ")));
    document.querySelectorAll("input[type=checkbox].opt").forEach(x => options.push(x.checked ? x.value : null));
    document.querySelectorAll("input[type=number].opt").forEach(x => options.push(x.dataset.opt, x.value));
    options.push("/tmp/input.peg");
    options = options.filter(x => x && x.length > 0);

    log("options:", options);
    log("input:", input.value.substr(0, 50));

    runBtn.disabled = true;
    runBtn.innerText = "Running ...";
    serializeState();
    stats("run", "Pegof was run");
    setTimeout(() => {
        Module.callMain(options);
        runBtn.disabled = false;
        runBtn.innerText = "Run!";
    }, 1);
}

function launchAuto() {
    log("launchAuto:", auto.checked, loaded, input.value != "");
    if (auto.checked && loaded && input.value != "") {
        launch();
    }
}

function loadPeg() {
    log("Loading", load.value);
    fetch(load.value)
        .then(response => response.text())
        .then(data => {
            input.innerHTML = data;
            peg = data;
            log("Loaded", load.value, data.substr(0, 50));
            stats("loaded-" + load.value, "Loaded " + load.value);
            launchAuto();
        });
}

function init() {
    deserializeState();

    load.addEventListener("change", loadPeg);
    auto.addEventListener("change", () => stats("auto-update-" + auto.checked, "Auto-update turned " + (auto.checked ? "on" : "off")));

    document.querySelectorAll(".opt").forEach(x => x.addEventListener("change", launchAuto));
    runBtn.addEventListener("click", launch);

    let timer = null;
    input.addEventListener("keydown", () => clearTimeout(timer));
    input.addEventListener("keyup", e => {
        if(e.key === "Enter" && (e.metaKey || e.ctrlKey)) {
            launch();
        } else {
            clearTimeout(timer);
            timer = setTimeout(launchAuto, 1000);
        }
    });

    document.addEventListener("DOMContentLoaded", function() {
    });
}

var Module = {
    noInitialRun: true,
    onRuntimeInitialized: () => {
        log("Runtime initialized");
        loaded = true;
        runBtn.disabled = false;
        runBtn.innerText = "Run!";
        launchAuto();
    },

    print: line => output.innerHTML += line + "\n",
    printErr: line => logs.innerHTML += line + "\n"
};

init();

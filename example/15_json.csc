// 15 — JSON
// JSON.stringify turns a native object into a JSON string;
// JSON.parse gives you a dynamic value whose properties resolve at runtime.

let config = {
    appName: "Cypescript IDE",
    version: "1.0.0",
    port: 8080,
    debug: false
};

// Object -> JSON string
let jsonStr: string = JSON.stringify(config);
println(jsonStr);   // {"appName":"Cypescript IDE","version":"1.0.0","port":8080,"debug":false}

// JSON string -> dynamic object
let parsed = JSON.parse(jsonStr);
println(parsed.appName);  // Cypescript IDE
println(parsed.port);     // 8080

// String-based JSON helpers from the stdlib also work
let doc: string = json_create_object();
doc = json_add_string(doc, "language", "Cypescript");
doc = json_add_int(doc, "year", 2026);
println(json_prettify(doc));

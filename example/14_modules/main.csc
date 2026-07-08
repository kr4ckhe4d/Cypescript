// 14 — Modules: the entry point
// Import paths are relative to this file; the .csc extension is optional.
// Run:  ./build/cscript -r example/14_modules/main.csc

import { square, cube, GREETING } from "./math_utils";

println(GREETING);              // hello from math_utils
println(square(7));             // 49
println(cube(3));               // 27
println(square(2) + cube(2));   // 12

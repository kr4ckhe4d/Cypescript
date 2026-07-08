// Tests: import/export module system (compile-time inlining)

import { square, cube, MODULE_GREETING } from "./modules/math_utils";

println(MODULE_GREETING); // hello from math_utils
println(square(7));       // 49
println(cube(3));         // 27

let combined: i32 = square(2) + cube(2);
println(combined);        // 12

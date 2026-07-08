// 04 — Control Flow
// if / else if / else, and switch/case/default (with fallthrough).

let score: i32 = 85;

if (score >= 90) {
    println("Grade: A");
} else if (score >= 80) {
    println("Grade: B");
} else if (score >= 70) {
    println("Grade: C");
} else {
    println("Grade: F");
}

// switch on integers — stack cases for fallthrough, break to exit
let day: i32 = 6;
switch (day) {
    case 1:
        println("Monday");
        break;
    case 6:
    case 7:
        println("Weekend!");
        break;
    default:
        println("Some weekday");
}

// switch also works on strings
let command: string = "stop";
switch (command) {
    case "start":
        println("starting...");
        break;
    case "stop":
        println("stopping...");
        break;
    default:
        println("unknown command");
}

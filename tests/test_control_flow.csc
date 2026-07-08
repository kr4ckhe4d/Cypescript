// Tests: break, continue, else-if chains, switch/case/default

// break + continue
let total: i32 = 0;
for (let i: i32 = 0; i < 10; i++) {
    if (i == 3) {
        continue;
    }
    if (i == 7) {
        break;
    }
    total += i;
}
println(total); // 18

// break in while
let count: i32 = 0;
while (true) {
    count = count + 1;
    if (count >= 5) {
        break;
    }
}
println(count); // 5

// else-if chains
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

// switch on integers with fallthrough and default
let day: i32 = 6;
switch (day) {
    case 1:
        println("Monday");
        break;
    case 6:
    case 7:
        println("Weekend");
        break;
    default:
        println("Weekday");
}

// switch on strings
let command: string = "stop";
switch (command) {
    case "start":
        println("starting");
        break;
    case "stop":
        println("stopping");
        break;
    default:
        println("unknown command");
}

// continue inside nested loop, break in switch inside loop
let matches: i32 = 0;
for (let i: i32 = 0; i < 5; i++) {
    switch (i) {
        case 2:
            matches += 1;
            break;
        default:
            break;
    }
}
println(matches); // 1

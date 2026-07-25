// The frame arena backs template literals and `+`. Without a game loop calling
// beginFrame() nothing is ever rewound, so this only checks that enabling the
// arena leaves string behaviour identical — the memory effect is asserted by
// tests/run_game_tests.sh, which measures RSS over a long headless run.

declare function enableFrameStrings(): void = "cyps_arena_enable";
declare function persist(text: string): string = "cyps_string_persist";

enableFrameStrings();

let score: i32 = 1234;
let ratio: f64 = 0.75;

println(`SCORE ${score}`);
println(`ratio ${ratio}`);
println("a" + "b" + "c");
println(`nested ${score} and ${ratio} together`);

// A string that must outlive its frame is copied out of the arena
let label: string = persist(`run ${score}`);
println(label);

// Arena strings still compare, concatenate and measure like any other string
let a: string = `x${score}`;
let b: string = "x1234";
if (a == b) { println("arena strings compare by value"); }
println(string_length(a));
println(string_upper(a));

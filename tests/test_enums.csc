// Enum members are folded to integers at parse time, so an enum costs nothing at
// runtime — it exists so key codes and entity kinds stop being magic numbers.

enum Color { Red, Green, Blue }
enum Key { Left = 263, Right = 262, Space = 32 }
enum Status { Ok = 0, Warn = 10, Error, Fatal }   // continues from the last explicit value
enum Offset { Behind = -1, Here, Ahead }

println(Color.Red);
println(Color.Green);
println(Color.Blue);
println(Key.Left);
println(Key.Space);
println(Status.Warn);
println(Status.Error);
println(Status.Fatal);
println(Offset.Behind);
println(Offset.Ahead);

// An enum name is a type, meaning i32
let c: Color = Color.Green;
println(c);

if (c == Color.Green) { println("compares"); }

// Usable in switch, arithmetic and arrays
switch (Color.Blue) {
    case Color.Red: println("red"); break;
    case Color.Blue: println("blue"); break;
    default: println("other");
}

println(Key.Left - Key.Right);

let palette: Color[] = [];
palette.push(Color.Red);
palette.push(Color.Blue);
println(palette[1]);

function describe(s: Status): string {
    if (s == Status.Fatal) { return "fatal"; }
    return "fine";
}
println(describe(Status.Fatal));

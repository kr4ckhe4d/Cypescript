// `i32[][]` is an array whose elements are array handles. Nested elements are
// stored verbatim rather than through the value paths, and the element type is
// resolved through the access chain — `grid[0][1]` used to fall back to i32 and
// read a pointer array as integers.

let grid: i32[][] = [];
let r: i32 = 0;
while (r < 3) {
    let row: i32[] = [];
    let c: i32 = 0;
    while (c < 4) { row.push(r * 10 + c); c += 1; }
    grid.push(row);
    r += 1;
}

println(grid.length);
println(grid[0].length);
println(grid[2][3]);

// Mutation through both levels, including compound assignment
grid[1][2] = 99;
println(grid[1][2]);
grid[1][2] += 1;
println(grid[1][2]);

// for...of over the rows
let total: i32 = 0;
for (const row of grid) {
    let i: i32 = 0;
    while (i < row.length) { total += row[i]; i += 1; }
}
println(total);

// Non-integer element types keep their own storage
let names: string[][] = [];
let firstRow: string[] = ["a", "b"];
names.push(firstRow);
println(names[0][1]);

let reals: f64[][] = [];
let realRow: f64[] = [1.5, 2.5];
reals.push(realRow);
println(reals[0][1]);

// Arrays of objects nested one level down
class Cell { v: i32 = 0; }
let cells: Cell[][] = [];
let cellRow: Cell[] = [];
cellRow.push(new Cell());
cells.push(cellRow);
cells[0][0].v = 7;
println(cells[0][0].v);

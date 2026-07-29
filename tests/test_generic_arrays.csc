// Test: element access on a T[] inside a generic function.
//
// Regression guard. The array runtime keeps a separate vector per element shape
// and dispatches on which one is written, so an element type has to name the
// same vector everywhere. `push` sent a generic element to the string vector
// while `arr[i]` read the i32 one, which is empty — so every indexed read
// silently produced nothing, with no error and no crash. `.length` hid it by
// reporting the max across all four vectors, so a loop bounded by length ran the
// right number of times over values that were not there.
//
// `shift()` and `for...of` always named the string vector, which is why they
// worked and indexing did not; they are asserted here alongside so the two
// cannot drift apart again.

function firstOf<T>(items: T[]): T {
    return items[0];
}

const words: string[] = ["alpha", "beta"];
println(firstOf<string>(words));

function exercise<T>(a: T, b: T): void {
    const queue: T[] = [];
    queue.push(a);
    queue.push(b);

    // Indexed reads: the values themselves, not just how many there are
    println(queue[0]);
    println(queue[1]);

    // Indexed store on a generic element
    queue[0] = b;
    println(queue[0]);

    // Walking by index, which is what a queue wants instead of an O(n) shift
    let head: i32 = 0;
    let counted: i32 = 0;
    while (head < queue.length) {
        const current: T = queue[head];
        head = head + 1;
        counted = counted + 1;
    }
    println(counted);

    // These two always worked; assert they still agree with indexing
    for (const item of queue) {
        println(item);
    }
    println(queue.shift()!);
    println(queue.length);
}

exercise<string>("alpha", "beta");

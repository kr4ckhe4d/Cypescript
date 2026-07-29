// BFS over a generated graph, exercising generics, Map, Set and array queues.
//
// Sizing note: the graph is built rather than hardcoded because the original
// six-node version ran in under a millisecond, which is the resolution floor of
// the user-time measurement run_benchmarks.sh reads — it reported a speedup that
// was pure measurement noise. The checksum below is printed so both languages can
// be shown to do identical work, and so the loop cannot be optimised away.
//
// This benchmark is what exposed `shift()` erasing from the front of a vector:
// at this size it took 0.79s against Node's 0.08s, ~91% of it in the queue. The
// runtime now advances a head offset instead, and the same 200,000 visits take
// 0.06s. Keep the graph around this size — large enough that the traversal is
// the subject, small enough that a regression shows up as time rather than as
// the machine swapping.

type Graph<T> = Map<T, T[]>;

function breadthFirstSearch<T>(graph: Graph<T>, startNode: T): T[] {
    const visited: Set<T> = new Set<T>();
    const queue: T[] = [];
    const traversalOrder: T[] = [];

    visited.add(startNode);
    queue.push(startNode);

    while (queue.length > 0) {
        const currentNode: T = queue.shift()!;
        traversalOrder.push(currentNode);

        const neighbors: T[] = graph.get(currentNode) || [];

        for (const neighbor of neighbors) {
            if (!visited.has(neighbor)) {
                visited.add(neighbor);
                queue.push(neighbor);
            }
        }
    }

    return traversalOrder;
}

let nodes: i32 = 5000;
let iterations: i32 = 40;

// Three out-edges per node, chosen so the graph is one connected component and
// every traversal reaches all of it — the doubling pair spreads the frontier, the
// +7 stride closes the cycles back up.
let graph: Map<string, string[]> = new Map<string, string[]>();
for (let i: i32 = 0; i < nodes; i = i + 1) {
    let neighbors: string[] = [];
    neighbors.push("N" + ((i * 2 + 1) % nodes));
    neighbors.push("N" + ((i * 2 + 2) % nodes));
    neighbors.push("N" + ((i + 7) % nodes));
    graph.set("N" + i, neighbors);
}

println("Starting Cypescript BFS benchmark (5,000 nodes, 40 traversals)...");

let checksum: i32 = 0;
for (let i: i32 = 0; i < iterations; i = i + 1) {
    let result: string[] = breadthFirstSearch<string>(graph, "N0");
    checksum = checksum + result.length;
}

println("Benchmark completed!");
println("Checksum (nodes visited): " + checksum);

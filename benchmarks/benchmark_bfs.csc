// BFS over a generated graph, exercising generics, Map, Set and array queues.
//
// Sizing note: the graph is built rather than hardcoded because the original
// six-node version ran in under a millisecond, which is the resolution floor of
// the user-time measurement run_benchmarks.sh reads — it reported a speedup that
// was pure measurement noise. The checksum below is printed so both languages can
// be shown to do identical work, and so the loop cannot be optimised away.
//
// `queue.shift()` is O(n) in both languages, so a large share of the time here is
// the queue rather than the traversal. That is a fair comparison — both sides run
// the same algorithm — but it is why the graph is 5,000 nodes and not 500,000:
// past a point this stops measuring BFS and starts measuring memmove.

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

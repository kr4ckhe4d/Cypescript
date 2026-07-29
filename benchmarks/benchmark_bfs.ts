// The Node reference for benchmark_bfs.csc. Keep the algorithm, the graph shape
// and the sizes identical to the .csc — the checksum both files print is what
// proves they did the same work.

function breadthFirstSearch(graph, startNode) {
    const visited = new Set();
    const queue = [];
    const traversalOrder = [];

    visited.add(startNode);
    queue.push(startNode);

    while (queue.length > 0) {
        const currentNode = queue.shift();
        traversalOrder.push(currentNode);

        const neighbors = graph.get(currentNode) || [];

        for (const neighbor of neighbors) {
            if (!visited.has(neighbor)) {
                visited.add(neighbor);
                queue.push(neighbor);
            }
        }
    }

    return traversalOrder;
}

const nodes = 5000;
const iterations = 40;

const graph = new Map();
for (let i = 0; i < nodes; i++) {
    const neighbors = [];
    neighbors.push("N" + ((i * 2 + 1) % nodes));
    neighbors.push("N" + ((i * 2 + 2) % nodes));
    neighbors.push("N" + ((i + 7) % nodes));
    graph.set("N" + i, neighbors);
}

console.log("Starting TypeScript/Node.js BFS benchmark (5,000 nodes, 40 traversals)...");

const start = Date.now();
let checksum = 0;
for (let i = 0; i < iterations; i++) {
    const result = breadthFirstSearch(graph, "N0");
    checksum += result.length;
}
const end = Date.now();

console.log("Benchmark completed!");
console.log(`Checksum (nodes visited): ${checksum}`);
console.log(`Execution time: ${end - start}ms`);

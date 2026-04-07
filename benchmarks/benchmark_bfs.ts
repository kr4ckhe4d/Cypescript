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

const graph = new Map();
graph.set("A", ["B", "C"]);
graph.set("B", ["D", "E"]);
graph.set("C", ["F"]);
graph.set("D", []);
graph.set("E", ["F"]);
graph.set("F", []);

const iterations = 1000;
console.log("Starting TypeScript/Node.js BFS benchmark (1,000 iterations)...");

const start = Date.now();
for (let i = 0; i < iterations; i++) {
    const result = breadthFirstSearch(graph, "A");
}
const end = Date.now();

console.log("Benchmark completed!");
console.log(`Execution time: ${end - start}ms`);

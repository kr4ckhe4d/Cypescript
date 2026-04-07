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

let graph: Map<string, string[]> = new Map<string, string[]>();
graph.set("A", ["B", "C"]);
graph.set("B", ["D", "E"]);
graph.set("C", ["F"]);
graph.set("D", []);
graph.set("E", ["F"]);
graph.set("F", []);

let iterations: i32 = 1000;
println("Starting Cypescript BFS benchmark (1,000 iterations)...");

for (let i: i32 = 0; i < iterations; i = i + 1) {
    let result: string[] = breadthFirstSearch<string>(graph, "A");
}

println("Benchmark completed!");

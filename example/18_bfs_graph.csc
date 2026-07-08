// 18 — Capstone: Breadth-First Search
// Generics, type aliases, Map/Set collections, arrays-as-queues, for...of.
// Compare with 18_bfs_graph.ts — the TypeScript source is nearly identical.

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

// Build the graph:  A -> B, C   B -> D, E   C -> F   E -> F
let graph: Map<string, string[]> = new Map<string, string[]>();
graph.set("A", ["B", "C"]);
graph.set("B", ["D", "E"]);
graph.set("C", ["F"]);
graph.set("D", []);
graph.set("E", ["F"]);
graph.set("F", []);

println("BFS traversal from 'A':");
let result: string[] = breadthFirstSearch<string>(graph, "A");
for (const node of result) {
    println(node);
}

// 18 — TypeScript version of 18_bfs_graph.csc, for comparison.
// Run with:  node example/18_bfs_graph.ts
// Note how little differs from the Cypescript version.

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

const graph: Graph<string> = new Map<string, string[]>();
graph.set("A", ["B", "C"]);
graph.set("B", ["D", "E"]);
graph.set("C", ["F"]);
graph.set("D", []);
graph.set("E", ["F"]);
graph.set("F", []);

console.log("BFS traversal from 'A':");
const result: string[] = breadthFirstSearch<string>(graph, "A");
for (const node of result) {
    console.log(node);
}

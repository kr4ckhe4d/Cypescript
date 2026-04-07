type Graph<T> = Map<T, T[]>;

/**
 * Performs a Breadth-First Search on a graph.
 * 
 * @param graph - The graph represented as an adjacency list.
 * @param startNode - The node to start the traversal from.
 * @returns An array of nodes in the order they were visited.
 */
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

// Example Usage:
let graph: Map<string, string[]> = new Map<string, string[]>();
graph.set("A", ["B", "C"]);
graph.set("B", ["D", "E"]);
graph.set("C", ["F"]);
graph.set("D", []);
graph.set("E", ["F"]);
graph.set("F", []);

println("Breadth-First Search Order starting from 'A':");
let result: string[] = breadthFirstSearch<string>(graph, "A");

for (const node of result) {
    println(node);
}

type Graph<T> = Map<T, T[]>;

function breadthFirstSearch<T>(graph: Graph<T>, startNode: T): T[] {
    const visited: Set<T> = new Set<T>();
    const queue: T[] = [];
    const traversalOrder: T[] = [];

    visited.add(startNode);
    println("Added startNode to visited");
    queue.push(startNode);
    println("Pushed startNode to queue");
    
    println("Queue length:");
    println(queue.length);

    while (queue.length > 0) {
        println("Queue length > 0");
        const currentNode: T = queue.shift()!;
        println("Processing node:");
        println(currentNode);
        traversalOrder.push(currentNode);

        const neighbors: T[] = graph.get(currentNode) || [];
        println("Neighbors found");

        for (const neighbor of neighbors) {
            println("Checking neighbor:");
            println(neighbor);
            if (!visited.has(neighbor)) {
                println("Visiting neighbor");
                visited.add(neighbor);
                queue.push(neighbor);
            }
        }
    }

    return traversalOrder;
}

// Test BFS
let graph: Map<string, string[]> = new Map<string, string[]>();
graph.set("A", ["B", "C"]);
graph.set("B", ["D"]);
graph.set("C", ["D"]);
graph.set("D", []);

println("BFS starting from A:");
let order: string[] = breadthFirstSearch<string>(graph, "A");

for (const node of order) {
    println(node);
}

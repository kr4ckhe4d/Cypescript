// Define a generic type for our Graph using an Adjacency List
type Graph<T> = Map<T, T[]>;

/**
 * Performs a Breadth-First Search on a graph.
 * * @param graph - The graph represented as an adjacency list.
 * @param startNode - The node where the search begins.
 * @returns An array containing the nodes in the order they were visited.
 */
function breadthFirstSearch<T>(graph: Graph<T>, startNode: T): T[] {
    // Edge case: If the start node isn't in the graph, return an empty array
    if (!graph.has(startNode)) {
        return [];
    }

    const visited = new Set<T>(); // Keeps track of nodes we've already seen
    const queue: T[] = [];        // FIFO queue to manage the traversal sequence
    const traversalOrder: T[] = []; // Stores the final order of visited nodes

    // Initialize the starting point
    visited.add(startNode);
    queue.push(startNode);

    // Loop until there are no more nodes left to explore in the queue
    while (queue.length > 0) {
        // Remove the first node from the queue (Dequeue)
        // Using '!' because we know the queue is not empty here
        const currentNode = queue.shift()!; 
        
        traversalOrder.push(currentNode);

        // Get all adjacent neighbors of the current node
        const neighbors = graph.get(currentNode) || [];

        // Check all neighbors
        for (const neighbor of neighbors) {
            if (!visited.has(neighbor)) {
                // Mark as visited before pushing to queue to avoid duplicate processing
                visited.add(neighbor);
                queue.push(neighbor);
            }
        }
    }

    return traversalOrder;
}
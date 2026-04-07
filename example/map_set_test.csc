let visited: Set<string> = new Set<string>();
visited.add("NodeA");
visited.add("NodeB");

println(visited.has("NodeA")); // 1
println(visited.has("NodeC")); // 0

let graph: Map<string, string[]> = new Map<string, string[]>();
let neighbors: string[] = ["NodeB", "NodeC"];
graph.set("NodeA", neighbors);

println(graph.has("NodeA")); // 1
let a_neighbors: string[] = graph.get("NodeA");
println(a_neighbors.length); // 2

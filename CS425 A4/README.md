# Distance Vector and Link State Routing Simulation

This C++ program simulates two fundamental network routing algorithms: Distance Vector Routing (DVR) and Link State Routing (LSR). It reads an adjacency matrix representing a network graph and computes the optimal routing tables for each node using both algorithms. The program outputs per-node routing tables for both DVR and LSR simulations.

## Features

- Distance Vector Routing (DVR):
  - Implements the Bellman-Ford-style iterative update method.
  - Each node updates its routing table based on neighbors' distance vectors.
  - Converges once no updates are made.
  - Tracks and prints the final cost and next-hop table for each node.

- Link State Routing (LSR):
  - Implements Dijkstra’s algorithm from each node to compute shortest paths.
  - Tracks the previous node (for path reconstruction) and determines next hops.
  - Uses a priority queue for efficient minimum cost node selection.

- Input/Output:
  - Accepts a file with an adjacency matrix as input.
  - Displays routing tables for all nodes under both DVR and LSR.
  - Supports unreachable links (9999) and no-links (0 for self-loops).

## Prerequisites

- C++ compiler (e.g., g++)
- Works on any standard Linux/macOS/Windows environment

## How to Compile and Run

### 1. Compile

g++ routing_sim.cpp -o routing_sim

### 2. Prepare the Input File

Create a text file (e.g., input.txt) with the following format:

4
0 10 100 30
10 0 20 40
100 20 0 10
30 40 10 0

- First line: Number of nodes n  
- Next n lines: Adjacency matrix where the i-th row and j-th column represent the cost from node i to node j  
- Use 9999 for unreachable links and 0 for self-links

### 3. Run the Program

./routing_sim input.txt

## Sample Output

--- Distance Vector Routing Simulation ---
Node 0 Routing Table:
Dest  Cost  Next Hop
0  0  -
1  10  1
2  30  3
3  30  3

...

--- Link State Routing Simulation ---
Node 0 Routing Table:
Dest  Cost  Next Hop
1  10  1
2  30  3
3  30  3

> Output includes routing tables for every node under both DVR and LSR.

## File Description

- routing_sim.cpp: Main simulation file implementing both DVR and LSR algorithms  
- input.txt: Example input file (user-provided)  
- README.md: This file containing build, run, and explanation details

## Contributors

All members contributed equally to design, implementation, and documentation.

- Rohan (220741)  
- Havish (220879)  
- Vijay (220602)

## Notes

- The adjacency matrix must be symmetric (undirected graph assumption).  
- The DVR section only prints the final routing table after convergence.  
  - For intermediate updates, insert printDVRTable() inside the DVR update loop.  
- The current implementation does not visualize the routing path; only final tables are printed.
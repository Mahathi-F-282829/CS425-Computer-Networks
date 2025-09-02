#include <iostream>
#include <vector>
#include <limits>
#include <queue>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

const int INF = 9999;

void printDVRTable(int node, const vector<vector<int>>& table, const vector<vector<int>>& nextHop) {
    cout << "Node " << node << " Routing Table:\n";
    cout << "Dest\tCost\tNext Hop\n";
    for (int i = 0; i < table.size(); ++i) {
        cout << i << "\t" << table[node][i] << "\t";
        if (nextHop[node][i] == -1) cout << "-";
        else cout << nextHop[node][i];
        cout << endl;
    }
    cout << endl;
}

void simulateDVR(const vector<vector<int>>& graph) {
    int n = graph.size();
    vector<vector<int>> dist = graph;
    vector<vector<int>> nextHop(n, vector<int>(n,-1));

    //TODO: Complete this
      // Initialize distance and nextHop matrices
for(int i=0; i<n; ++i) {
    for(int j=0; j<n; ++j) {
        if(i == j) {
            // Distance to self is always 0; no next hop needed
            dist[i][j] = 0;
            nextHop[i][j] = -1;
        }
        else if(graph[i][j] == INF) {
            // No direct link: set distance to INF and next hop to -1 (unreachable)
            dist[i][j] = INF;
            nextHop[i][j] = -1;
        }
        else {
            // Direct neighbor: set distance to direct cost and next hop to neighbor
            dist[i][j] = graph[i][j];
            nextHop[i][j] = j;
        }
    }
}

/*
 * Main Distance Vector Routing (DVR) update loop.
 * Each node updates its routing table by considering all possible intermediate nodes (k).
 * If a shorter path to destination j via neighbor k is found, update the cost and next hop.
 * Repeat this process until no updates occur (convergence).
 */
bool updated;
do {
    updated = false;

    // Make copies to store new distances and next hops for this iteration
    vector<vector<int>> newDist = dist;
    vector<vector<int>> newNextHop = nextHop;

    // For each node i (source)
    for(int i=0; i<n; ++i) {
        // For each possible destination j
        for(int j=0; j<n; ++j) {
            if(i == j) continue; // Skip self

            int minCost = newDist[i][j];    // Current known minimum cost from i to j
            int minHop = newNextHop[i][j];  // Current known next hop from i to j

            // Try all possible neighbors k as intermediate nodes
            for(int k=0; k<n; ++k) {
                if(k == i || dist[i][k] == INF) continue; // Skip self and unreachable neighbors

                int newCost = dist[i][k] + dist[k][j]; // Cost to go from i to j via k

                // If a shorter path is found via neighbor k
                if(newCost < minCost) {
                    minCost = newCost;
                    minHop = k;
                }
            }

            // If a shorter path was found, update the new tables and mark as updated
            if(minCost < newDist[i][j]) {
                newDist[i][j] = minCost;
                newNextHop[i][j] = minHop;
                updated = true;
            }
        }
    }

    // If any updates were made in this iteration, copy new tables for the next round
    if(updated) {
        dist = newDist;
        nextHop = newNextHop;
    }
} while(updated); // Repeat until no updates are made (convergence)

   

    cout << "--- DVR Final Tables ---\n";
    for (int i = 0; i < n; ++i) printDVRTable(i, dist, nextHop);
}

void printLSRTable(int src, const vector<int>& dist, const vector<int>& prev) {
    cout << "Node " << src << " Routing Table:\n";
    cout << "Dest\tCost\tNext Hop\n";
    for (int i = 0; i < dist.size(); ++i) {
        if (i == src) continue;
        cout << i << "\t" << dist[i] << "\t";
        int hop = i;
        while (prev[hop] != src && prev[hop] != -1)
            hop = prev[hop];
        cout << (prev[hop] == -1 ? -1 : hop) << endl;
    }
    cout << endl;
}

void simulateLSR(const vector<vector<int>>& graph) {
    int n = graph.size();
    for (int src = 0; src < n; ++src) {
        vector<int> dist(n, INF);
        vector<int> prev(n, -1);
        vector<bool> visited(n, false);
        dist[src] = 0;
        
         //TODO: Complete this
         // Create a min-heap priority queue to select the node with the smallest distance
priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> pq;

// Start from the source node with distance 0
pq.push({0, src});

while(!pq.empty()) {
    // Get the node u with the smallest current distance (cur_dist)
    auto [cur_dist, u] = pq.top();
    pq.pop();
    
    // If node u has already been visited, skip it
    if(visited[u]) continue;
    visited[u] = true; // Mark node u as visited

    // For every possible neighbor v of node u
    for(int v=0; v<n; ++v) {
        // Skip self-loops and unreachable neighbors
        if(v == u || graph[u][v] == INF) continue;
        
        // Calculate the new distance to v via u
        int new_dist = cur_dist + graph[u][v];

        // If this path is shorter, update the distance and predecessor
        if(new_dist < dist[v]) {
            dist[v] = new_dist; // Update shortest distance to v
            prev[v] = u;        // Set u as the predecessor of v
            pq.push({new_dist, v}); // Add v to the priority queue with updated distance
        }
    }
}

        
        printLSRTable(src, dist, prev);
    }
}

vector<vector<int>> readGraphFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        exit(1);
    }
    
    int n;
    file >> n;
    vector<vector<int>> graph(n, vector<int>(n));

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            file >> graph[i][j];

    file.close();
    return graph;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }

    string filename = argv[1];
    vector<vector<int>> graph = readGraphFromFile(filename);

    cout << "\n--- Distance Vector Routing Simulation ---\n";
    simulateDVR(graph);

    cout << "\n--- Link State Routing Simulation ---\n";
    simulateLSR(graph);

    return 0;
}

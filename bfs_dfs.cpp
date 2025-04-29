#include <iostream>
#include <vector>
#include <queue>
#include <omp.h>

using namespace std;

class Graph {
    int V;
    vector<vector<int>> adj;

public:
    Graph(int vertices) {
        V = vertices;
        adj.resize(V);
    }

    void addEdge(int u, int v) {
        adj[u].push_back(v);  // Undirected
        adj[v].push_back(u);
    }

    void parallelBFS(int start) {
        vector<bool> visited(V, false);
        queue<int> q;

        visited[start] = true;
        q.push(start);

        cout << "\nParallel BFS Traversal: ";

        while (!q.empty()) {
            int level_size = q.size();
            vector<int> next_level;

            #pragma omp parallel
            {
                vector<int> local_nodes;
                #pragma omp for nowait
                for (int i = 0; i < level_size; ++i) {
                    int node;

                    #pragma omp critical
                    {
                        node = q.front();
                        q.pop();
                        cout << node << " ";
                    }

                    for (int neighbor : adj[node]) {
                        if (!visited[neighbor]) {
                            #pragma omp critical
                            {
                                if (!visited[neighbor]) {
                                    visited[neighbor] = true;
                                    local_nodes.push_back(neighbor);
                                }
                            }
                        }
                    }
                }

                #pragma omp critical
                next_level.insert(next_level.end(), local_nodes.begin(), local_nodes.end());
            }

            for (int node : next_level)
                q.push(node);
        }

        cout << endl;
    }

    void parallelDFSUtil(int node, vector<bool>& visited) {
        #pragma omp critical
        cout << node << " ";

        visited[node] = true;

        #pragma omp parallel for
        for (int i = 0; i < adj[node].size(); ++i) {
            int neighbor = adj[node][i];
            if (!visited[neighbor]) {
                #pragma omp task firstprivate(neighbor)
                parallelDFSUtil(neighbor, visited);
            }
        }

        #pragma omp taskwait
    }

    void parallelDFS(int start) {
        vector<bool> visited(V, false);
        cout << "\nParallel DFS Traversal: ";
        #pragma omp parallel
        {
            #pragma omp single
            parallelDFSUtil(start, visited);
        }
        cout << endl;
    }
};

int main() {
    int vertices, edges, u, v, start;

    cout << "Enter number of vertices: ";
    cin >> vertices;

    cout << "Enter number of edges: ";
    cin >> edges;

    Graph g(vertices);

    cout << "Enter each edge (u v):\n";
    for (int i = 0; i < edges; ++i) {
        cin >> u >> v;
        g.addEdge(u, v);
    }

    cout << "Enter starting node for traversal: ";
    cin >> start;

    g.parallelBFS(start);
    g.parallelDFS(start);

    return 0;
}

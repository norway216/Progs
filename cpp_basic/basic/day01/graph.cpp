#include <iostream>
#include <vector>
#include <queue>

class Graph {
public:
    // 构造函数，指定顶点数量
    Graph(int vertices) : m_vertices(vertices) {
        m_adj.resize(vertices);
    }

    // 添加无向边
    void addEdge(int u, int v) {
        if (!isValidVertex(u) || !isValidVertex(v)) {
            std::cout << "Invalid vertex: " << u << " or " << v << std::endl;
            return;
        }

        m_adj[u].push_back(v);
        m_adj[v].push_back(u);
    }

    // BFS 遍历
    void bfs(int start) const {
        if (!isValidVertex(start)) {
            std::cout << "Invalid start vertex: " << start << std::endl;
            return;
        }

        std::vector<bool> visited(m_vertices, false);
        std::queue<int> q;

        visited[start] = true;
        q.push(start);

        std::cout << "BFS traversal starting from vertex " << start << ": ";

        while (!q.empty()) {
            int current = q.front();
            q.pop();

            std::cout << current << " ";

            for (int neighbor : m_adj[current]) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    q.push(neighbor);
                }
            }
        }

        std::cout << std::endl;
    }

    // 打印图的邻接表
    void printGraph() const {
        std::cout << "Adjacency List:" << std::endl;
        for (int i = 0; i < m_vertices; ++i) {
            std::cout << i << " -> ";
            for (int neighbor : m_adj[i]) {
                std::cout << neighbor << " ";
            }
            std::cout << std::endl;
        }
    }

private:
    int m_vertices;
    std::vector<std::vector<int>> m_adj;

    bool isValidVertex(int v) const {
        return v >= 0 && v < m_vertices;
    }
};

int main() {
    // 创建一个有 6 个顶点的图：0~5
    Graph graph(6);

    graph.addEdge(0, 1);
    graph.addEdge(0, 2);
    graph.addEdge(1, 3);
    graph.addEdge(1, 4);
    graph.addEdge(2, 5);

    graph.printGraph();
    graph.bfs(0);

    return 0;
}
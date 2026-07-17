#include <iostream>
#include <map>
#include <vector>

class Edge;

enum class VertexType {
    Combine,
    Sequence,
};

struct Vertex {
    VertexType         type;
    std::vector<Edge*> inEdges;
    std::vector<Edge*> outEdges;
};

struct Edge {
    Vertex* from;
    Vertex* to;
};

struct Graph {
    std::vector<Vertex*> vertices;
    std::vector<Edge*>   edges;
};

class TopoIter {
    Graph* g_;

public:
    TopoIter(Graph* g) : g_(g) { Init(); }
    bool HasNext() {
        if (index < start.size()) {
            return true;
        }
        start.clear();
        if (!next.empty()) {
            index = 0;
            start.swap(next);
            return true;
        }
        std::vector<std::vector<Vertex*>> vertexs;
        for (auto [v, count] : inDegree) {
            if (count == 0) {
                continue;
            }
            vertexs.resize(count);
            vertexs[count - 1].push_back(v);
        }
        for (auto& vs : vertexs) {
            if (vs.empty()) {
                continue;
            }
            start.swap(vs);
            index = 0;
            break;
        }
        return !start.empty();
    }

    Vertex* Next() {
        auto* v = start[index++];
        if (v->type == VertexType::Combine) {
            return v;
        }
        ComputeNext(v);
        return v;
    }

private:
    std::vector<Vertex*>   start;
    std::vector<Vertex*>   next;
    std::map<Vertex*, int> inDegree;
    int                    index = 0;
    void                   Init() {
        for (auto v : g_->vertices) {
            if (v->type == VertexType::Sequence || !v->inEdges.empty()) {
                start.push_back(v);
                continue;
            }
            inDegree[v] = v->inEdges.size();
        }
    }

    void ComputeNext(Vertex* v) {
        for (auto e : v->outEdges) {
            auto* to = e->to;
            if (to->type == VertexType::Sequence) {
                continue;
            }
            if (--inDegree[to] == 0) {
                next.push_back(to);
            }
        }
    }
};

int main() { return 0; }
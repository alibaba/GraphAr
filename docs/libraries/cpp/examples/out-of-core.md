# Out-of-core Graph Algorithms

An important application case of GraphAr is to serve out-of-core graph
processing scenarios. With the graph data saved as GraphAr format files in the
disk, GraphAr provides a set of reading interfaces to allow to load part
of graph data into memory when needed, to conduct analytics. While it is
more convenient and efficient to store the entirety of the graph in
memory (as is done in BGL), out-of-core graph processing makes it
possible to complete analytics on the large-scale graphs using limited
memory/computing resources.

The are some out-of-core graph analytic algorithms that have been
implemented based on GraphAr, include:

- PageRank (PR)
- Connected Components (CC)
- Breadth First Search (BFS)

These algorithms represent for different compute patterns and are
usually building blocks for constructing other graph algorithms.

## PageRank

[PageRank (PR)](https://en.wikipedia.org/wiki/PageRank) is an algorithm
used by Google Search to rank web pages in their search engine results.
The source code of PageRank based on GraphAr located at
[pagerank_example.cc](https://github.com/apache/incubator-graphar/blob/main/cpp/examples/pagerank_example.cc),
and the explanations can be found in the [Getting
Started](../getting-started#a-pagerank-example) page.

## Connected Components

A weakly connected component is a maximal subgraph of a graph such that
for every pair of vertices in it, there is an undirected path connecting
them. And [Connected Components
(CC)](https://en.wikipedia.org/wiki/Connected_component) is an algorithm
to identify all weakly connected components in a graph. [CC based on
BGL](./bgl) is provided in GraphAr, also, we implement out-of-core
algorithms for this application.

A typical method for calculating CC is label propagation. In this
algorithm, each vertex is attached with a property which represents its
component label, being its own vertex id initially. In the subsequent
supersteps (i.e., iterations), a vertex will update its label if it
receives a smaller id and then it propagates this id to all its
neighbors.

This algorithm can be implemented based on streaming the edges via
GraphAr's reading interface. That is to say, the edges are accessed and
processed chunk by chunk, instead of being loaded into memory at once
(as in the BGL example).

```c++
// construct the edge collection in GraphAr
auto edges = ...
auto it_begin = edges->begin(), it_end = edges->end();

// initialize for all vertices
std::vector<graphar::IdType> component(num_vertices);
for (graphar::IdType i = 0; i < num_vertices; i++)
  component[i] = i;

// stream all edges for each iteration
for (int iter = 0; ; iter++) {
  bool flag = false;
  for (auto it = it_begin; it != it_end; ++it) {
    graphar::IdType src = it.source(), dst = it.destination();
    // update
    if (component[src] < component[dst]) {
        component[dst] = component[src];
        flag = true;
    } else if (component[src] > component[dst]) {
        component[src] = component[dst];
        flag = true;
    }
  }
  // check if it should terminate
  if (!flag) break;
}
```

The file
[cc_stream_example.cc](https://github.com/apache/incubator-graphar/blob/main/cpp/examples/cc_stream_example.cc)
located inside the source tree contains the complete implementation for
this algorithm. Also, we can only process active vertices (the vertices
which are updated in the last iteration) and the corresponding edges for
each iteration, since an inactive vertex does not need to update its
neighbors. Please refer to
[cc_push_example.cc](https://github.com/apache/incubator-graphar/blob/main/cpp/examples/cc_push_example.cc)
for the complete code.

:::tip

In this example, two kinds of edges are used. The
**ordered_by_source** edges are used to access all outgoing edges of
an active vertex, and **ordered_by_dest** edges are used to access the
incoming edges. In this way, all the neighbors of an active vertex can
be accessed and processed.

Although GraphAr supports to get the outgoing (incoming) edges of a
single vertex for all adjList types, it is most efficient when the
type is **ordered_by_source** (**ordered_by_dest**) since it can avoid
to read redundant data.

:::

## Breadth First Search

[Breadth First Search
(BFS)](https://en.wikipedia.org/wiki/Breadth-first_search) is a
traversing algorithm that starts from a selected vertex (the root) and
traverse the graph layer-wise thus exploring the neighbor vertices
(vertices which are directly connected to the root), and then it moves
towards the next-level neighbor vertices.

An out-of-core BFS algorithm could be implemented based on streaming the
graph data via GraphAr. For each vertex, a property named *distance* is
created and initialized to represent the distance from the root to this
vertex. As with the standard BFS algorithms in other graph processing
systems, for the iteration/superstep *i* (starting from 0), the active
vertices contain all vertices reachable from the root in *i* hops (i.e.,
*distance\[v\]= i*). At the beginning of the algorithm, only the root
vertex is active. This algorithm terminates when there are no more
active vertices.

```c++
// construct the edge collection in GraphAr
auto edges = ...
auto it_begin = edges->begin(), it_end = edges->end();

// initialize for all vertices
graphar::IdType root = 0; // the BFS root
std::vector<int32_t> distance(num_vertices);
for (graphar::IdType i = 0; i < num_vertices; i++)
  distance[i] = (i == root ? 0 : -1);

// stream all edges for each iteration
for (int iter = 0; ; iter++) {
  graphar::IdType count = 0;
  for (auto it = it_begin; it != it_end; ++it) {
    graphar::IdType src = it.source(), dst = it.destination();
    // update
    if (distance[src] == iter && distance[dst] == -1) {
      distance[dst] = distance[src] + 1;
      count++;
    }
  }
  // check if it should terminate
  if (count == 0) break;
}
```

The above algorithm is implemented based on streaming all edges for each
iteration, the source code can be found at
[bfs_stream_example.cc](https://github.com/apache/incubator-graphar/blob/main/cpp/examples/bfs_stream_example.cc).

Meanwhile, BFS could be implemented in a **push**-style which only
traverses the edges that from active vertices for each iteration, which
is typically more efficient on real-world graphs. This implementation
can be found at
[bfs_push_example.cc](https://github.com/apache/incubator-graphar/blob/main/cpp/examples/bfs_push_example.cc).
Similarly, we provide a BFS implementation in a **pull**-style which
only traverses the edges that lead to non-visited vertices (i.e., the
vertices that have not been traversed), as shown in
[bfs_pull_example.cc](https://github.com/apache/incubator-graphar/blob/main/cpp/examples/bfs_pull_example.cc).

:::tip

In common cases of graph processing, the **push**-style is more
efficient when the set of active vertices is very sparse, while the
**pull**-style fits when it is dense.

:::

In some cases, it is required to record the path of BFS, that is, to
maintain each vertex's predecessor (also called *father*) in the
traversing tree rather than only recording the distance. The
implementation of BFS with recording fathers can be found at
[bfs_father_example.cc](https://github.com/apache/incubator-graphar/blob/main/cpp/examples/bfs_father_example.cc).

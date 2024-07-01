---
id: getting-started
title: Getting Started
sidebar_position: 1
---


# Getting Started

This article is a quick guide that explains how to work with GraphAr
C++. To begin with, please refer to the [Building
Steps](https://github.com/apache/incubator-graphar/blob/main/README.md#building-libraries)
to install GraphAr.

## Information Files

GraphAr uses a group of Yaml files to save the meta information for a graph.

### Graph information

The graph information file defines the most basic information of a graph
includes its name, the root directory path of the data files, the vertex
information and edge information files it contains, and the version of
GraphAr. For example, the file "ldbc_sample.graph.yml" defines an
example graph named "ldbc_sample", which includes one type of vertices
("person") and one type of edges ("person knows person").

``` Yaml
name: ldbc_sample
prefix: ./
vertices:
  - person.vertex.yml
edges:
  - person_knows_person.edge.yml
version: gar/v1
```

### Vertex information

Each vertex information file defines a single group of vertices with the
same vertex label, e.g., "person" in this case. The vertex chunk size,
the relative path for vertex data files and the version of GraphAr are
specified. These vertices could have some properties, which are divided
into property groups. Each property group has its own file type (CSV,
ORC or Parquet) and the prefix of the relative path for its data files,
it also lists all properties in this group, with every property contains
the name, data type and if it is the primary key.

The file
[person.vertex.yml](https://github.com/apache/incubator-graphar-testing/blob/main/ldbc_sample/csv/person.vertex.yml)
located inside the test data contains an example of the vertex
information file. In this example, the "person" vertices have two
property groups. The first group contains only one property (named "id")
and the second group contains three properties ("firstName", "lastName"
and "gender").

### Edge information

Each edge information file defines a single type of edges with specific
labels for the source vertex, destination vertex and the edge, e.g.,
"person_knows_person" in this case. It defines the meta information such
as the edge chunk size, the source vertex chunk size, the destination
vertex chunk size, if the edges are directed or not, the relative file
path for edge data files, the adjLists and the version of GraphAr. The
file
[person_knows_person.edge.yml](https://github.com/apache/incubator-graphar-testing/blob/main/ldbc_sample/csv/person_knows_person.edge.yml)
located inside the test data contains an example of the edge information
file.

In GraphAr format, separate data files are used to store the structure
(called adjList) and the properties for edges. The adjList type can be
either of **unordered_by_source**, **unordered_by_dest**,
**ordered_by_source** or **ordered_by_dest**. For a specific
type of adjList, the meta information includes its file path prefix, the
file type, as well as all the property groups attached.

:::note

It is allowed to store different types of adjLists for a group of
edges at the same time.

:::

## Data Files

### Property data

The vertex properties are stored in vertex property chunks with the
chunk size specified by the vertex information file. Different property
groups correspond to individual groups of data files. In our example,
the property group ("first name", "last name", "gender") for vertex
chunk 0 of "person" vertices are stored in
[./vertex/person/firstName_lastName_gender/chunk0](https://github.com/apache/incubator-graphar-testing/blob/main/ldbc_sample/csv/vertex/person/firstName_lastName_gender/chunk0).

In practice of graph processing, it is common to only query a subset of
columns of the properties. Thus, the column-oriented formats like Apache
ORC and Apache Parquet are more efficient, which eliminate the need to
read columns that are not relevant. We also provide data files in ORC
and Parquet for the example graph in the [test
data](https://github.com/apache/incubator-graphar-testing/blob/main/ldbc_sample/).

Similar with vertices, the edge properties are stored in edge property
chunks. For each vertex chunk, its associated edges (if the edge type is
**ordered_by_source** or **unordered_by_source**, the associated edges
are those in which the source vertex is in that chunk; otherwise, if the
edge type is **ordered_by_dest** or **unordered_by_dest**, the
associated edges are those in which the destination is in that chunk)
are maintained in some edge chunks, with the size of each chunk not
exceeding the edge chunk size specified in the edge information file.

For instance, the file
[./edge/person_knows_person/ordered_by_source/creationDate/part0/chunk0](https://github.com/apache/incubator-graphar-testing/blob/main/ldbc_sample/csv/edge/person_knows_person/ordered_by_source/creationDate/part0/chunk0)
stores the property group "creationDate" of "person_knows_person" edges
for the first edge chunk of the first vertex chunk, and the adjList type
of the edges is **ordered_by_source**.

### AdjList data

The adjList in GraphAr describes the topology structure, i.e., the internal
id of the source vertex and the destination vertex for each of a group
of edges. As explained in [Edges in GraphAr](https://graphar.apache.org/docs/specification/format#edge-chunks-in-graphar), the edges are separated
into edge chunks, and each edge chunk has its own adjList table and 0 or
more property tables.

For example, the file
[./edge/person_knows_person/ordered_by_source/adj_list/part0/chunk0](https://github.com/apache/incubator-graphar-testing/blob/main/ldbc_sample/csv/edge/person_knows_person/ordered_by_source/adj_list/part0/chunk0)
saves the adjList of "person_knows_person" edges for the first edge
chunk of the first vertex chunk, and the adjList type of the edges is
"ordered_by_source". This adjList table consists of only two columns:
one for the source and one for the destination; it can be saved in CSV,
ORC, or Parquet files.

:::note

If the edges are ordered, there may also be offset chunks to construct
the index for accessing edges of a single vertex. These chunks will
tore the start offset of each vertex's edges, see
[./edge/person_knows_person/ordered_by_source/offset/chunk0](https://github.com/apache/incubator-graphar-testing/blob/main/ldbc_sample/csv/edge/person_knows_person/ordered_by_source/offset/chunk0)
as an example.

:::

## How to Use GraphAr

### Construct information

It is convenient to construct the graphar metadata and dump it to generate
information files. We provide an [example
program](https://github.com/apache/incubator-graphar/blob/main/cpp/examples/construct_info_example.cc)
located in the source code which shows how to construct and dump the
files for graph information, vertex information and edge information.

Also, the metadata of a graph can be constructed easily through reading
the already existed information files, as the following code
illustrates:

``` C++
// construct graph information from file
std::string path = ... // the path of the graph information file (e.g., ldbc_sample.graph.yml)
auto graph_info = graphar::GraphInfo::Load(path).value();

// get vertex information
auto vertex_info = graph_info->GetVertexInfo("person");
if (vertex_info != nullptr) {
  // use vertex_info ...
}

// get edge information
auto edge_info = graph_info->GetEdgeInfo("person", "knows", "person");
if (edge_info != nullptr) {
  // use edge_info ...
}
```

### Read GraphAr format files

GraphAr supports the flexible reading of graph data, e.g., allowing to read
data of a single vertex, a vertex chunk, or all vertices with a specific
label. In addition, necessary property groups can be selected to read
and avoid reading all properties from the files. Furthermore, GraphAr
provides convenient and flexible access to adjList, offset and property
chunks for edges.

As a simple case, the following example shows how to read all vertices
with label "person" of the graph defined by "graph_info" and output the
values of "id" and "firstName" for each vertex.

``` C++
graph_info = ...
auto vertices = graphar::VerticesCollection::Make(graph_info, "person").value();

for (auto it = vertices->begin(); it != vertices->end(); ++it) {
  // get a vertex and access its data
  auto vertex = *it;
  if (vertex.IsValid("id") && vertex.IsValid("firstName"))
    std::cout << "id=" << vertex.property<int64_t>("id").value() << ", firstName=" << vertex.property<std::string>("firstName").value() << std::endl;
}
```

The next example reads all edges with label "person_knows_person" from
the above graph and outputs the end vertices for each edge.

``` C++
graph_info = ...
auto expect = graphar::EdgesCollection::Make(graph_info, "person", "knows", "person", graphar::AdjListType::ordered_by_source);
auto edges = expect.value();

for (auto it = edges->begin(); it != edges->end(); ++it) {
  // get an edge and access its data
  auto edge = *it;
  std::cout << "src=" << edge.source() << ", dst=" << edge.destination() << std::endl;
}
```

### Write GraphAr format files

As same with the readers, the GraphAr writers provide different-level
methods to output the graph data in memory into GraphAr format files.

As the simplest cases, the fist example below adds vertices to
**VerticesBuilder**, and then dumps the data to files; the second
example constructs a collection of edges and then dumps them.

``` C++
vertex_info = ...
prefix = ...
graphar::builder::VerticesBuilder builder(vertex_info,  prefix);

// add a vertex
graphar::builder::Vertex v;
v.AddProperty("id", 933);
v.AddProperty("firstName", "Alice");
builder.AddVertex(v);
// add other vertices
// ...

// write to GraphAr format files
builder.Dump();
```

``` C++
edge_info = ...
prefix = ...
vertices_num = ...
graphar::builder::EdgesBuilder builder(edge_info, prefix, graphar::AdjListType::ordered_by_source, vertices_num);

// add an edge (0 -> 3)
graphar::builder::Edge e(0, 3);
e.AddProperty("creationDate", "2011-07-20T20:02:04.233+0000");
builder.AddEdge(e);
// add other edges
// ...

// write to GraphAr format files
builder.Dump();
```

### A PageRank Example

Here we will go through an example of out-of-core graph analytic
algorithms based on GraphAr which calculates the PageRank. Please look
[here](https://en.wikipedia.org/wiki/PageRank) if you want a detailed
explanation of the PageRank algorithm. And the source code can be found
at
[pagerank_example.cc](https://github.com/apache/incubator-graphar/blob/main/cpp/examples/pagerank_example.cc).

This program first reads in the graph information file to obtain the
metadata; then, it constructs the vertex and edge collections to enable
access to the graph. After that, an implementation of the PageRank
algorithm is provided, with data for the vertices stored in memory, and
the edges streamed through disk I/O. Finally, the vertex information
with type "person" is extended to include a new property named
"pagerank" (a new vertex information file named
*person-new-pagerank.vertex.yml* is saved) and the **VerticesBuilder**
is used to write the results to new generated data chunks.

Please refer to [more examples](examples/out-of-core.md) to learn
about the other available case studies utilizing GraphAr.

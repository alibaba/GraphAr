Getting Started
============================

This article is a quick guide that explains how to work with GraphAr. To begin with, please refer to the `Building Steps`_ to install GraphAr. After reading this article to gain a basic understanding of GraphAr, move on to `GraphAr File Format <file-format.html>`_ to explore more specific explanations of the file format, or `the C++ library <../reference/api-reference-cpp.html>`_ and `the Spark library <spark-lib.html>`_  to learn about the GraphAr libraries.


GAR Information Files
------------------------

GAR uses a group of Yaml files to save the meta information for a graph.

Graph information
`````````````````
The graph information file defines the most basic information of a graph includes its name, the root directory path of the data files, the vertex information and edge information files it contains, and the version of GraphAr. For example, the file "ldbc_sample.graph.yml" defines an example graph named "ldbc_sample", which includes one type of vertices ("person") and one type of edges ("person knows person").

.. code:: Yaml

  name: ldbc_sample
  prefix: ./
  vertices:
    - person.vertex.yml
  edges:
    - person_knows_person.edge.yml
  version: 1

Vertex information
``````````````````
Each vertex information file defines a single group of vertices with the same vertex label, e.g., "person" in this case. The vertex chunk size, the relative path for vertex data files and the version of GraphAr are specified. These vertices could have some properties, which are divided into property groups. Each property group has its own file type (CSV, ORC or Parquet) and the prefix of the relative path for its data files, it also lists all properties in this group, with every property contains the name, data type and if it is the primary key.

The file `person.vertex.yml`_ located inside the test data contains an example of the vertex information file. In this example, the "person" vertices have two property groups. The first group contains only one property (named "id") and the second group contains three properties ("firstName", "lastName" and "gender").

Edge information
````````````````
Each edge information file defines a single type of edges with specific labels for the source vertex, destination vertex and the edge, e.g., "person_konws_person" in this case. It defines the meta information such as the edge chunk size, the source vertex chunk size, the destination vertex chunk size, if the edges are directed or not, the relative file path for edge data files, the adjLists and the version of GraphAr. The file `person_knows_person.edge.yml`_ located inside the test data contains an example of the edge information file.

In GAR format, separate data files are used to store the structure (called adjList) and the properties for edges. The adjList type can be either of **unordered_by_source**, **unordered_by_dest**, **ordered_by_source** or **ordered_by_dest** (see `Edges in GraphAr <file-format.html#edges-in-graphar>`_ for more). For a specific type of adjList, the meta information includes its file path prefix, the file type, as well as all the property groups attached.

.. note::

  It is allowed to store different types of adjLists for a group of edges at the same time.



GAR Data Files
------------------------

Property data
`````````````
The vertex properties are stored in vertex property chunks with the chunk size specified by the vertex information file. Different property groups correspond to individual groups of data files.
In our example, the property group ("first name", "last name", "gender") for vertex chunk 0 of "person" vertices are stored in `./vertex/person/firstName_lastName_gender/chunk0`_.

In practice of graph processing, it is common to only query a subset of columns of the properties. Thus, the column-oriented formats like Apache ORC and Apache Parquet are more efficient, which eliminate the need to read columns that are not relevant. We also provide data files in ORC and Parquet for the example graph in the `test data`_.

Similar with vertices, the edge properties are stored in edge property chunks. For each vertex chunk, its associated edges (if the edge type is **ordered_by_source** or **unordered_by_source**, the associated edges are those in which the source vertex is in that chunk; otherwise, if the edge type is **ordered_by_dest** or **unordered_by_dest**, the associated edges are those in which the destination is in that chunk) are maintained in some edge chunks, with the size of each chunk not exceeding the edge chunk size specified in the edge information file.

For instance, the file `./edge/person_knows_person/ordered_by_source/creationDate/part0/chunk0`_ stores the property group "creationDate" of "person_knows_person" edges for the first edge chunk of the first vertex chunk, and the adjList type of the edges is **ordered_by_source**.

AdjList data
````````````
The adjList in GAR describes the topology structure, i.e., the source vertex id and the destination vertex id for each of a group of edges. As explained in `Edges in GraphAr <file-format.html#edges-in-graphar>`_, the edges are separated into edge chunks, and each edge chunk has its own adjList table and 0 or more property tables.

For example, the file `./edge/person_knows_person/ordered_by_source/adj_list/part0/chunk0`_ saves the adjList of "person_knows_person" edges for the first edge chunk of the first vertex chunk, and the adjList type of the edges is "ordered_by_source". This adjList table consists of only two columns: one for the source and one for the destination; it can be saved in CSV, ORC, or Parquet files.

.. note::

  If the edges are ordered, there may also be offset chunks to construct the index for accessing edges of a single vertex. These chunks will store the start offset of each vertex's edges, see `./edge/person_knows_person/ordered_by_source/offset/chunk0`_ as an example.


How to Use GAR
------------------------

Construct information
`````````````````````
It is convenient to construct the GAR metadata and dump it to generate information files. We provide an `example program`_ located in the source code which shows how to construct and dump the files for graph information, vertex information and edge information.

Also, the metadata of a graph can be constructed easily through reading the already existed information files, as the following code illustrates:

.. code:: C++

  // construct graph information from file
  std::string path = ... // the path of the graph information file (e.g., ldbc_sample.graph.yml)
  auto graph_info = GraphArchive::GraphInfo::Load(path).value();

  // get vertex information
  auto maybe_vertex_info = graph_info.GetVertexInfo("person");
  if (maybe_vertex_info.status().ok())) {
    auto vertex_info = maybe_vertex_info.value();
    // use vertex_info ...
  }

  // get edge information
  auto& maybe_edge_info = graph_info.GetEdgeInfo("person", "knows", "person");
  if (maybe_edge_info.status().ok())) {
    auto edge_info = maybe_vertex_info.value();
    // use edge_info ...
  }


Read GAR files
``````````````
GAR supports the flexible reading of graph data, e.g., allowing to read data of a single vertex, a vertex chunk, or all vertices with a specific label. In addition, necessary property groups can be selected to read and avoid reading all properties from the files. Furthermore, GAR provides convenient and flexible access to adjList, offset and property chunks for edges.

As a simple case, the following example shows how to read all vertices with label "person" of the graph defined by "graph_info" and output the values of "id" and "firstName" for each vertex.

.. code:: C++

  graph_info = ...
  auto& vertices = GraphArchive::ConstructVerticesCollection(graph_info, "person").value();

  for (auto it = vertices.begin(); it != vertices.end(); ++it) {
    // get a vertex and access its data
    auto vertex = *it;
    std::cout << "id=" << vertex.property<int64_t>("id").value() << ", firstName=" << vertex.property<std::string>("firstName").value() << std::endl;
  }

The next example reads all edges with label "person_knows_person" from the above graph and outputs the end vertices for each edge.

.. code:: C++

  graph_info = ...
  auto expect = GraphArchive::ConstructEdgesCollection(graph_info, "person", "konws" "person", GraphArchive::AdjListType::ordered_by_source).value();
  auto& edges = std::get<GraphArchive::EdgesCollection<GraphArchive::AdjListType::ordered_by_source>>(expect.value());

  for (auto it = edges.begin(); it != edges.end(); ++it) {
    // get an edge and access its data
    auto edge = *it;
    std::cout << "src=" << edge.source() << ", dst=" << edge.destination() << std::endl;
  }

See also `C++ Reader API Reference <../reference/api-reference-cpp.html#readers>`_.

Write GAR files
```````````````
As same with the readers, the GAR writers provide different-level methods to output the graph data in memory into GAR files.

As the simplest cases, the fist example below adds vertices to **VerticesBuilder**, and then dumps the data to files; the second example constructs a collection of edges and then dumps them.

.. code:: C++

  vertex_info = ...
  prefix = ...
  GraphArchive::builder::VerticesBuilder builder(vertex_info,  prefix);

  // add a vertex
  GraphArchive::builder::Vertex v;
  v.AddProperty("id", 933);
  v.AddProperty("firstName", "Alice");
  builder.AddVertex(v);
  // add other vertices
  // ...

  // write to GAR files
  builder.Dump();

.. code:: C++

  edge_info = ...
  prefix = ...
  GraphArchive::builder::EdgesBuilder builder(edge_info, prefix, GraphArchive::AdjListType::ordered_by_source);

  // add an edge (0 -> 3)
  GraphArchive::builder::Edge e(0, 3);
  e.AddProperty("creationDate", "2011-07-20T20:02:04.233+0000");
  builder.AddVertex(e);
  // add other edges
  // ...

  // write to GAR files
  builder.Dump();

See also `C++ Writer API Reference <../reference/api-reference-cpp.html#writer-and-builder>`_.

A PageRank Example
``````````````````
Here we will go through an example of out-of-core graph analytic algorithms based on GAR which calculates the PageRank. Please look `here <https://en.wikipedia.org/wiki/PageRank>`_ if you want a detailed explanation of the PageRank algorithm. And the source code can be found at `pagerank_example.cc`_.

This program first reads in the graph information file to obtain the metadata; then, it constructs the vertex and edge collections to enable access to the graph. After that, an implementation of the PageRank algorithm is provided, with data for the vertices stored in memory, and the edges streamed through disk I/O. Finally, the vertex information with type "person" is extended to include a new property named "pagerank" (a new vertex information file named *person-new-pagerank.vertex.yml* is saved) and the **VerticesBuilder** is used to write the results to new generated data chunks.

Please refer to `more examples <../applications/out-of-core.html>`_ to learn about the other available case studies utilizing GraphAr.

.. _Building Steps: https://github.com/alibaba/GraphAr/blob/main/README.rst

.. _person.vertex.yml: https://github.com/GraphScope/gar-test/blob/main/ldbc_sample/csv/person.vertex.yml

.. _person_knows_person.edge.yml: https://github.com/GraphScope/gar-test/blob/main/ldbc_sample/csv/person_knows_person.edge.yml

.. _./vertex/person/firstName_lastName_gender/chunk0: https://github.com/GraphScope/gar-test/blob/main/ldbc_sample/csv/vertex/person/firstName_lastName_gender/chunk0

.. _test data: https://github.com/GraphScope/gar-test/blob/main/ldbc_sample/

.. _./edge/person_knows_person/ordered_by_source/creationDate/part0/chunk0: https://github.com/GraphScope/gar-test/blob/main/ldbc_sample/csv/edge/person_knows_person/ordered_by_source/creationDate/part0/chunk0

.. _./edge/person_knows_person/ordered_by_source/adj_list/part0/chunk0: https://github.com/GraphScope/gar-test/blob/main/ldbc_sample/csv/edge/person_knows_person/ordered_by_source/adj_list/part0/chunk0

.. _./edge/person_knows_person/ordered_by_source/offset/chunk0: https://github.com/GraphScope/gar-test/blob/main/ldbc_sample/csv/edge/person_knows_person/ordered_by_source/offset/chunk0

.. _example program: https://github.com/alibaba/GraphAr/blob/main/examples/construct_info_example.cc

.. _pagerank_example.cc: https://github.com/alibaba/GraphAr/blob/main/examples/pagerank_example.cc

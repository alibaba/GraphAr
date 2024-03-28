Co-Work with BGL
============================

The `Boost Graph Library (BGL) <https://cs.brown.edu/~jwicks/boost/libs/graph/doc/>`_  is the first C++ library to apply the principles of generic programming to the construction of the advanced data structures and algorithms used in graph computations. The BGL graph interface and graph components are generic in the same sense as the Standard Template Library (STL). And it provides some built-in algorithms which cover a core set of algorithm patterns and a larger set of graph algorithms.

We take calculating CC as an example, to demonstrate how BGL works with GraphAr. A weakly connected component is a maximal subgraph of a graph such that for every pair of vertices in it, there is an undirected path connecting them. And the CC algorithm is to identify all such components in a graph. Learn more about `the CC algorithm <https://en.wikipedia.org/wiki/Connected_component>`_.

The source code of CC based on BGL can be found at `bgl_example.cc`_. In this program, the graph information file is first read to get the metadata:

.. code:: C++

   std::string path = ... // the path of the graph information file
   auto graph_info = graphar::GraphInfo::Load(path).value();

And then, the vertex collection and the edge collection are established as the handles to access the graph data:

.. code:: C++

   auto maybe_vertices = graphar::VerticesCollection::Make(graph_info, "person");
   auto vertices = maybe_vertices.value();
   auto maybe_edges = graphar::EdgesCollection::Make(graph_info, "person", "knows", "person", graphar::AdjListType::ordered_by_source);
   auto edges = maybe_edges.value();

Next, we construct the in-memory graph data structure for BGL by traversing the vertices and edges via GraphAr's high-level reading interface (the vertex iterator and the edge iterator):

.. code:: C++

   // define the Graph type in BGL
   typedef boost::adjacency_list<boost::vecS, // use vector to store edges
                                 boost::vecS, // use vector to store vertices
                                 boost::undirectedS, // undirected
                                 boost::property<boost::vertex_name_t, int64_t>, // vertex property
                                 boost::no_property> Graph; // no edge property
   // descriptors for vertex in BGL
   typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;

   // declare a graph object with (num_vertices) vertices and an edge iterator
   std::vector<std::pair<graphar::IdType, graphar::IdType>> edges_array;
   auto it_begin = edges->begin(), it_end = edges->end();
   for (auto it = it_begin; it != it_end; ++it)
      edges_array.push_back(std::make_pair(it.source(), it.destination()));
   Graph g(edges_array.begin(), edges_array.end(), num_vertices);

   // define the internal vertex property "id"
   boost::property_map<Graph, boost::vertex_name_t>::type id = get(boost::vertex_name_t(), g);
   auto v_it_begin = vertices->begin(), v_it_end = vertices->end();
   for (auto it = v_it_begin; it != v_it_end; ++it) {
      auto vertex = *it;
      boost::put(id, vertex.id(), vertex.property<int64_t>("id").value());
   }

After that, an internal CC algorithm provided by BGL is called:

.. code:: C++

   // define the external vertex property "component"
   std::vector<int> component(num_vertices);
   // call algorithm: cc
   int cc_num = boost::connected_components(g, &component[0]);
   std::cout << "Total number of components: " << cc_num << std::endl;

Finally, we could use a **VerticesBuilder** of GraphAr to write the results to new generated GAR files:

.. code:: C++

   // construct a new property group
   graphar::Property cc = {"cc", graphar::int32(), false};
   std::vector<graphar::Property> property_vector = {cc};
   auto group = graphar::CreatePropertyGroup(property_vector, graphar::FileType::PARQUET);

   // construct the new vertex info
   std::string vertex_label = "cc_result", vertex_prefix = "result/";
   int chunk_size = 100;
   auto new_info = graphar::CreateVertexInfo(vertex_label, chunk_size, {group}, vertex_prefix);

   // access the vertices via the index map and vertex iterator of BGL
   typedef boost::property_map<Graph, boost::vertex_index_t>::type IndexMap;
   IndexMap index = boost::get(boost::vertex_index, g);
   typedef boost::graph_traits<Graph>::vertex_iterator vertex_iter;
   std::pair<vertex_iter, vertex_iter> vp;

   // dump the results through the VerticesBuilder
   graphar::builder::VerticesBuilder builder(new_info, "/tmp/");
   for (vp = boost::vertices(g); vp.first!= vp.second; ++vp.first) {
      Vertex v = *vp.first;
      graphar::builder::Vertex vertex(index[v]);
      vertex.AddProperty(cc.name, component[index[v]]);
      builder.AddVertex(vertex);
   }
   builder.Dump();


.. _bgl_example.cc: https://github.com/alibaba/GraphAr/blob/main/cpp/examples/bgl_example.cc

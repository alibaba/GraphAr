Convert SNAP Datasets to GraphAr Format
=======================================

`SNAP <https://snap.stanford.edu/data/>`_ (Stanford Network Analysis Project) is a general-purpose network analysis and graph mining library. It provides a variety of datasets for research and development. In this section, we will show how to convert the SNAP datasets to GraphAr format, showcasing the process with the `ego-Facebook <https://snap.stanford.edu/data/ego-Facebook.html>`_ graph as a case study. The conversion leverages GraphInfo constructors and the high-level writer functions from the C++ library.


Prepare the SNAP Dataset
------------------------

Before converting, download the ego-Facebook dataset from the SNAP website. The dataset is a text file with each line representing an edge in the graph.

.. code:: bash

  cd /path/to/your/dataset
  wget https://snap.stanford.edu/data/facebook_combined.txt.gz
  gunzip facebook_combined.txt.gz


Convert the SNAP Dataset to GraphAr Format
------------------------------------------

The initial phase involves constructing VertexInfo, EdgeInfo, and GraphInfo objects, which are subsequently serialized into YAML files. 
For instance, the code snippet below illustrates the creation and storage of the vertex information file.

.. code:: C++

  auto version = graphar::InfoVersion::Parse("gar/v1").value();

  // meta info
  std::string vertex_label = "node", vertex_prefix = "vertex/node/";

  // create vertex info
  auto vertex_info = graphar::CreateVertexInfo(
      vertex_label, VERTEX_CHUNK_SIZE, {}, vertex_prefix, version);

  // save & dump vertex info
  ASSERT(!vertex_info->Dump().has_error());
  ASSERT(vertex_info->Save(save_path + "node.vertex.yml").ok());

  // create and save edge info file ...
  auto edge_info = ...
  ASSERT(!edge_info->Dump().has_error());
  ASSERT(edge_info->Save(save_path + "node_links_node.edge.yml").ok());

  // create and save graph info file ...
  auto graph_info = ...
  ASSERT(!graph_info->Dump().has_error());
  ASSERT(graph_info->Save(save_path + graph_name + ".graph.yml").ok());
 

Subsequently, we employ the high-level vertex and edge builders provided by the GraphAr C++ library to generate payload data files with vertex and edge data. 
The code snippet that follows demonstrates the generation and preservation of the edge data file.

.. code:: C++

  // construct edges builder
  auto e_builder = graphar::builder::EdgesBuilder::Make(
                       edge_info, save_path, ADJLIST_TYPE, VERTEX_COUNT)
                       .value();
  // read edge data from file
  std::ifstream file(DATA_PATH);
  std::string line;
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    // skip comments
    if (line[0] == '#') {
      continue;
    }
    int src, dst;
    if (!(iss >> src >> dst)) {
      break;
    }
    graphar::builder::Edge e(src, dst);
    ASSERT(e_builder->AddEdge(e).ok());
  }

  // dump & clear
  ASSERT(e_builder->Dump().ok());
  e_builder->Clear();

For comprehensive insights into this example, please consult the accompanying `source code <https://github.com/alibaba/GraphAr/tree/main/docs/cpp/examples/snap_dataset_to_graphar.cc>`_ .

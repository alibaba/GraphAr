GraphAr Spark Library
============================

Overview
-----------

The GraphAr Spark library is provided for generating, loading and transforming GAR files with Apache Spark easy. It consists of several components:

- **Information Classes**: As same with in the C++ library, the information classes are implemented as a part of the Spark library for constructing and accessing the meta information about the graphs, vertices and edges in GraphAr.
- **IndexGenerator**: The IndexGenerator helps to generate the indices for vertex/edge DataFrames. In most cases, IndexGenerator is first utilized to generate the indices for a DataFrame (e.g., from primary keys), and then this DataFrame can be written into GAR files through the writer.
- **Writer**: The GraphAr Spark writer provides a set of interfaces that can be used to write Spark DataFrames into GAR files. Every time it takes a DataFrame as the logical table for a type of vertices or edges, assembles the data in specified format (e.g., reorganize the edges in the CSR way) and then dumps it to standard GAR files (CSV, ORC or Parquet files) under the specific directory path.
- **Reader**: The GraphAr Spark reader provides a set of interfaces that can be used to read GAR files. It reads a collection of vertices or edges at a time and assembles the result into the Spark DataFrame. Similar with the reader in the C++ library, it supports the users to specify the data they need, e.g., reading a single property group instead of all properties.

Use Cases
----------

The GraphAr Spark library can be used in a range of scenarios:

- Taking GAR as a data source to execute SQL queries or do graph processing (e.g., using GraphX).
- Transforming data between GAR and other data sources (e.g., Hive, Neo4j, NebulaGraph, ...).
- Transforming GAR data between different file types (e.g., from ORC to Parquet).
- Transforming GAR data between different adjList types (e.g., from COO to CSR).
- Modifying existing GAR data (e.g., adding new vertices/edges).


Get GraphAr Spark Library
------------------------------

Make the graphar-spark-library directory as the current working directory:

.. code-block:: shell

   cd spark/

Compile package:

.. code-block:: shell

   mvn package

After compilation, a similar file *graphar-0.1.0-SNAPSHOT-shaded.jar* is generated in the directory *spark/target/*.


How to Use
-----------------

Information classes
`````````````````````
The Spark library for GraphAr provides distinct information classes for constructing and accessing meta information about graphs, vertices, and edges. These classes act as essential parameters for constructing readers and writers, and they can be built either from the existing meta files (in the Yaml format) or in-memory from scratch.

To construct information from a Yaml file, please refer to the following example code.

.. code:: scala

   // read graph yaml and construct information
   val file_path = ... // the path to the yaml file
   val yaml_path = new Path(file_path)
   val fs = FileSystem.get(yaml_path.toUri(), spark.sparkContext.hadoopConfiguration)
   val input = fs.open(yaml_path)
   val graph_yaml = new Yaml(new Constructor(classOf[GraphInfo]))
   val graph_info = graph_yaml.load(input).asInstanceOf[GraphInfo]

   // use information classes
   val vertices = graph_info.getVertices
   val edges = graph_info.getEdges
   val version = graph_info.getVersion

See `TestGraphInfo.scala`_ for the complete example.


IndexGenerator
``````````````````
The GraphAr file format assigns each vertex with a unique index (vertex id) starting from 0 and increasing continuously for each type of vertex (i.e., with the same vertex label). However, the vertex/edge tables in Spark often lack this information, requiring special attention. For example, an edge table typically uses the primary key (e.g., "id", which is a string) to identify its source and destination vertices.

To address this issue, the GraphAr Spark library offers the IndexGenerator which is used to generate indices for vertex/edge DataFrames. For a vertex DataFrame, a mapping from the primary keys to GAR indices can be constructed, or an index column can be generated directly if no primary keys are available. For an edge DataFrame, source and destination columns can be generated from the vertex index mapping (when the end vertices are represented by the primary keys), or they may be generated directly without the mapping.

.. tip::
   In most cases, IndexGenerator is first utilized to generate the indices for a DataFrame, and then this DataFrame can be written into GAR files through the writer.

To utilize IndexGenerator, please refer to the following example code.

.. code:: scala

   // generate indices for vertex DataFrame
   val vertex_df = ...
   val vertex_df_with_index = IndexGenerator.generateVertexIndexColumn(vertex_df)

   // generate indices for src & dst columns of edge DataFrame
   val edge_df = ...
   val edge_df_with_index = IndexGenerator.generateSrcAndDstIndexUnitedlyForEdges(edge_df, "src", "dst")

   // generate indices for src & dst columns of edge DataFrame from vertex primary keys
   val vertex_df = ...
   val edge_df = ...
   val vertex_mapping = IndexGenerator.constructVertexIndexMapping(vertex_df, "id")
   val edge_df_src_index = IndexGenerator.generateSrcIndexForEdgesFromMapping(edge_df, "src", vertex_mapping)
   val edge_df_src_dst_index = IndexGenerator.generateDstIndexForEdgesFromMapping(edge_df_src_index, "dst", vertex_mapping)

See `TestIndexGenerator.scala`_ for the complete example.


Writer
``````````````````
The GraphAr Spark writer provides the necessary Spark interfaces to write DataFrames into GraphAr formatted files in a batch-import fashion. With the VertexWriter, users can specify a particular property group to be written into its corresponding chunks, or choose to write all property groups. For edge chunks, besides the meta data (edge info), the adjList type should also be specified. The adjList/properties can be written alone, or alternatively, all adjList, properties, and the offset (for CSR and CSC format) chunks can be written simultaneously.

To utilize the GAR Spark writer, please refer to the following example code.

.. code:: scala

   // generate the vertex index column for vertex dataframe
   val vertex_df = ...
   val vertex_df_with_index = IndexGenerator.generateVertexIndexColumn(vertex_df)
   // construct the vertex writer
   val vertex_info = ...
   val prefix = ...
   val writer = new VertexWriter(prefix, vertex_info, vertex_df_with_index)
   // write certain property group
   val property_group = vertex_info.getPropertyGroup("id")
   writer.writeVertexProperties(property_group)
   // write all properties
   writer.writeVertexProperties()

   // generate vertex index for edge dataframe
   val edge_df = ...
   val edge_df_with_index = IndexGenerator.generateSrcAndDstIndexUnitedlyForEdges(edge_df, "src", "dst")
   // construct the edge writer
   val edge_info = ...
   val adj_list_type = AdjListType.ordered_by_source
   val writer = new EdgeWriter(prefix, edge_info, adj_list_type, edge_df_with_index)
   // write adjList
   writer.writeAdjList()
   // write certain property group
   val property_group = edge_info.getPropertyGroup("creationDate", adj_list_type)
   writer.writeEdgeProperties(property_group)
   // write all of adjList and properties
   writer.writeEdges()

See `TestWriter.scala`_ for the complete example.


Reader
``````````````````
The GraphAr Spark reader provides an extensive set of interfaces to read GAR files. It reads a collection of vertices or edges at a time and assembles the result into the Spark DataFrame. Similar with the reader in C++ library, it supports the users to specify the data they need, e.g., a single property group.

After content has been read into the Spark DataFrame, users can leverage it to do graph processing, execute SQL queries or perform various transformations (such as adding new vertices/edges, reorganizing the edge order, and changing the file type) and then write it back into GAR files if desired.

To utilize the GAR Spark reader, please refer to the following example code.

.. code:: scala

   // construct the vertex reader
   val prefix = ...
   val vertex_info = ...
   val reader = new VertexReader(prefix, vertex_info, spark)
   val property_group = vertex_info.getPropertyGroup("gender")
   // read a single chunk
   val single_chunk_df = reader.readVertexPropertyChunk(property_group, 0)
   // ...
   // read all property chunks
   val vertex_df = reader.readAllVertexProperties()

   //construct the edge reader
   val edge_info = ...
   val adj_list_type = AdjListType.ordered_by_source
   val reader = new EdgeReader(prefix, edge_info, adj_list_type, spark)
   // read a single adjList chunk
   val single_adj_list_df = reader.readAdjListChunk(2, 0)
   // read all adjList chunks for a vertex chunk
   val adj_list_df_chunk_2 = reader.readAdjListForVertexChunk(2)
   // ...
   // read all edge chunks (including adjList and all properties)
   val edge_df = reader.readEdges()

See `TestReader.scala`_ for the complete example.

More examples
``````````````````
For more information on usage, please refer to the examples:

- `ComputeExample.scala`_  includes an example for constructing the GraphX graph from GAR files and executing a connected-components computation.
- `TransformExample.scala`_ shows an example for graph data conversion between different file types or different adjList types.


.. _TestGraphInfo.scala: https://github.com/alibaba/GraphAr/blob/main/spark/src/test/scala/com/alibaba/graphar/TestGraphInfo.scala

.. _TestIndexGenerator.scala: https://github.com/alibaba/GraphAr/blob/main/spark/src/test/scala/com/alibaba/graphar/TestIndexGenerator.scala

.. _TestWriter.scala: https://github.com/alibaba/GraphAr/blob/main/spark/src/test/scala/com/alibaba/graphar/TestWriter.scala

.. _TestReader.scala: https://github.com/alibaba/GraphAr/blob/main/spark/src/test/scala/com/alibaba/graphar/TestReader.scala

.. _ComputeExample.scala: https://github.com/alibaba/GraphAr/blob/main/spark/src/test/scala/com/alibaba/graphar/ComputeExample.scala

.. _TransformExample.scala: https://github.com/alibaba/GraphAr/blob/main/spark/src/test/scala/com/alibaba/graphar/TransformExample.scala

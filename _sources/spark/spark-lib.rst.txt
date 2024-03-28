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

For more information on its usage, please refer to the `Examples <examples/spark.html>`_.


Get GraphAr Spark Library
------------------------------

Building from source
`````````````````````

Make the graphar-spark-library directory as the current working directory:

.. code-block:: shell

   cd GraphAr/spark/

Compile package:

.. code-block:: shell

   mvn clean package -DskipTests

GraphAr supports two Apache Spark versions for now and uses Maven Profiles to work with it. The command above built GrahpAr with Spark 3.2.2 by default. To built GraphAr with Spark 3.3.4 use the following command:

.. code-block:: shell

   mvn clean package -DskipTests -P datasources-33

After compilation, a similar file *graphar-x.x.x-SNAPSHOT-shaded.jar* is generated in the directory *spark/graphar/target/*.

Please refer to the `building steps <https://github.com/alibaba/GraphAr/tree/main/spark>`_ for more details.

Get from Maven
```````````````

You can include GraphAr as a dependency in your maven project

.. code-block:: shell

   <repositories>
      <repository>
         <id>graphar-mvn-repo</id>
         <url>https://github.com/alibaba/GraphAr/raw/mvn-repo/</url>
      </repository>
   </repositories>
   <dependencies>
      <dependency>
         <groupId>org.apache</groupId>
         <artifactId>graphar</artifactId>
         <version>0.1.0</version>
      </dependency>
   </dependencies>


How to Use
-----------------

Information classes
`````````````````````
The Spark library for GraphAr provides distinct information classes for constructing and accessing meta information about graphs, vertices, and edges. These classes act as essential parameters for constructing readers and writers, and they can be built either from the existing meta files (in the Yaml format) or in-memory from scratch.

To construct information from a Yaml file, please refer to the following example code.

.. code-block:: scala

   // read graph yaml and construct information
   val spark = ... // the Spark session
   val file_path = ... // the path to the yaml file
   val graph_info = GraphInfo.loadGraphInfo(file_path, spark)

   // use information classes
   val vertices = graph_info.getVertices
   val edges = graph_info.getEdges
   val version = graph_info.getVersion

See `TestGraphInfo.scala`_ for the complete example.


IndexGenerator
``````````````````
The GraphAr file format assigns each vertex with a unique index inside the vertex type (which called internal vertex id) starting from 0 and increasing continuously for each type of vertex (i.e., with the same vertex label). However, the vertex/edge tables in Spark often lack this information, requiring special attention. For example, an edge table typically uses the primary key (e.g., "id", which is a string) to identify its source and destination vertices.

To address this issue, the GraphAr Spark library offers the IndexGenerator which is used to generate indices for vertex/edge DataFrames. For a vertex DataFrame, a mapping from the primary keys to GAR indices can be constructed, or an index column can be generated directly if no primary keys are available. For an edge DataFrame, source and destination columns can be generated from the vertex index mapping (when the end vertices are represented by the primary keys), or they may be generated directly without the mapping.

.. tip::
   In most cases, IndexGenerator is first utilized to generate the indices for a DataFrame, and then this DataFrame can be written into GAR files through the writer.

To utilize IndexGenerator, please refer to the following example code.

.. code-block:: scala

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

.. code-block:: scala

   // generate the vertex index column for vertex DataFrame
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

   // generate vertex index for edge DataFrame
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

.. code-block:: scala

   // construct the vertex reader
   val prefix = ...
   val vertex_info = ...
   val reader = new VertexReader(prefix, vertex_info, spark)
   val property_group = vertex_info.getPropertyGroup("gender")
   // read a single chunk
   val single_chunk_df = reader.readVertexPropertyChunk(property_group, 0)
   // ...
   // read all property chunks
   val vertex_df = reader.readAllVertexPropertyGroups()

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


Graph-level APIs
``````````````````
To improve the usability of the GraphAr Spark library, a set of APIs are provided to allow users to easily perform operations such as reading, writing, and transforming data at the graph level. These APIs are fairly easy to use, while the previous methods of using reader, writer and information classes are more flexibly and can be highly customized.

The Graph Reader is a helper object which enables users to read all the chunk files from GraphAr for a single graph. The only input required is a GraphInfo object or the path to the information yaml file. On successful completion, it returns a set of vertex DataFrames and edge DataFrames, each of which can be accessed by specifying the vertex/edge label. The Graph Writer is used for writing all vertex DataFrames and edge DataFrames of a graph to generate GraphAr chunk files. For more details, please refer to the `API Reference <../reference/spark-api/index.html>`_ .

The Graph Transformer is a helper object in the GraphAr Spark library, designed to assist with data transformation at the graph level. It takes two GraphInfo objects (or paths of two yaml files) as inputs: one for the source graph, and one for the destination graph. The transformer will then load data from existing GAR files for the source graph, utilizing the GraphAr Spark Reader and the meta data defined in the source GraphInfo. After reorganizing the data according to the destination GraphInfo, it generates new GAR chunk files with the GraphAr Spark Writer.

.. code-block:: scala

   // transform graphs by yaml paths
   val spark = ... // the Spark session
   val source_path = ... // e.g., /tmp/source.graph.yml
   val dest_path = ... // e.g., /tmp/dest.graph.yml
   GraphTransformer.transform(source_path, dest_path, spark)

   // transform graphs by information objects
   val source_info = ...
   val dest_info = ...
   GraphTransformer.transform(source_info, dest_info, spark)


We provide an example in `TestGraphTransformer.scala`_, which demonstrates how to conduct data transformation from the `source graph <https://github.com/GraphScope/gar-test/blob/main/ldbc_sample/parquet/ldbc_sample.graph.yml>`_ to the `destination graph <https://github.com/GraphScope/gar-test/blob/main/transformer/ldbc_sample.graph.yml>`_.

The Graph Transformer can be used for various purposes, including transforming GAR data between different file types (e.g. from ORC to Parquet), transforming between different adjList types (e.g. from COO to CSR), selecting properties or regrouping them, and setting a new chunk size.

.. note::
   There are certain limitations while using the Graph Transformer:

   -  The vertices (or edges) of the source and destination graphs are aligned by labels, meaning each vertex/edge label included in the destination graph must have an equivalent in the source graph, in order for the related chunks to be loaded as the data source.
   -  For each group of vertices/edges (i.e., each single label), each property included in the destination graph (defined in the relevant VertexInfo/EdgeInfo) must also be present in the source graph.

   In addition, users can use the GraphAr Spark Reader/Writer to conduct data transformation more flexibly at the vertex/edge table level, as opposed to the graph level. This allows for a more granular approach to transforming data, as `TransformExample.scala`_ shows.


More examples
``````````````````
For more information on usage, please refer to the examples:

- `ComputeExample.scala`_  includes an example for constructing the GraphX graph from GAR files and executing a connected-components computation.
- `TransformExample.scala`_ shows an example for graph data conversion between different file types or different adjList types.
- `Neo4j2GraphAr.scala`_ and `GraphAr2Neo4j.scala`_ are examples to conduct data importing/exporting for Neo4j.


.. _TestGraphInfo.scala: https://github.com/alibaba/GraphAr/blob/main/spark/src/test/scala/com/alibaba/graphar/TestGraphInfo.scala

.. _TestIndexGenerator.scala: https://github.com/alibaba/GraphAr/blob/main/spark/src/test/scala/com/alibaba/graphar/TestIndexGenerator.scala

.. _TestWriter.scala: https://github.com/alibaba/GraphAr/blob/main/spark/src/test/scala/com/alibaba/graphar/TestWriter.scala

.. _TestReader.scala: https://github.com/alibaba/GraphAr/blob/main/spark/src/test/scala/com/alibaba/graphar/TestReader.scala

.. _TestGraphTransformer.scala: https://github.com/alibaba/GraphAr/blob/main/spark/src/test/scala/com/alibaba/graphar/TestGraphTransformer.scala

.. _ComputeExample.scala: https://github.com/alibaba/GraphAr/blob/main/spark/src/test/scala/com/alibaba/graphar/ComputeExample.scala

.. _TransformExample.scala: https://github.com/alibaba/GraphAr/blob/main/spark/src/test/scala/com/alibaba/graphar/TransformExample.scala

.. _Neo4j2GraphAr.scala: https://github.com/alibaba/GraphAr/blob/main/spark/src/main/scala/com/alibaba/graphar/example/Neo4j2GraphAr.scala

.. _GraphAr2Neo4j.scala: https://github.com/alibaba/GraphAr/blob/main/spark/src/main/scala/com/alibaba/graphar/example/GraphAr2Neo4j.scala

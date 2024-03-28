GraphAr Java Library
====================

Overview
--------

Based on an efficient FFI for Java and C++ called
`fastFFI <https://github.com/alibaba/fastFFI>`__, the GraphAr Java
library allows users to write Java for generating, loading and
transforming GAR files. It consists of several components:

-  **Information Classes**: As same with in the C++ library, the
   information classes are implemented to construct and access the meta
   information about the **graphs**, **vertices** and **edges** in
   GraphAr.

-  **Writers**: The GraphAr Java writer provides a set of interfaces
   that can be used to write Apache Arrow VectorSchemaRoot into GAR
   files. Every time it takes a VectorSchemaRoot as the logical table
   for a type of vertices or edges, then convert it to ArrowTable, and
   then dumps it to standard GAR files (CSV, ORC or Parquet files) under
   the specific directory path.

-  **Readers**: The GraphAr Java reader provides a set of interfaces
   that can be used to read GAR files. It reads a collection of vertices
   or edges at a time and assembles the result into the ArrowTable.
   Similar with the reader in the C++ library, it supports the users to
   specify the data they need, e.g., reading a single property group
   instead of all properties.

Get GraphAr Java Library
------------------------

Building from source
~~~~~~~~~~~~~~~~~~~~

Only support installing from source currently, but we will support
installing from Maven in the future.

Firstly, install llvm-11. ``LLVM11_HOME`` should point to the home of
LLVM 11. In Ubuntu, it is at ``/usr/lib/llvm-11``. Basically, the build
procedure the following binary:

-  ``$LLVM11_HOME/bin/clang++``

-  ``$LLVM11_HOME/bin/ld.lld``

-  ``$LLVM11_HOME/lib/cmake/llvm``

Tips:

-  Use Ubuntu as example:

   .. code-block:: bash

      $ sudo apt-get install llvm-11 clang-11 lld-11 libclang-11-dev libz-dev -y
      $ export LLVM11_HOME=/usr/lib/llvm-11

-  Or compile from source with this
   `script <https://github.com/alibaba/fastFFI/blob/main/docker/install-llvm11.sh>`__:

   .. code-block:: bash

      $ export LLVM11_HOME=/usr/lib/llvm-11
      $ export LLVM_VAR=11.0.0
      $ sudo ./install-llvm11.sh

Make the graphar-java-library directory as the current working
directory:

.. code-block:: bash

    $ git clone https://github.com/alibaba/GraphAr.git
    $ cd GraphAr
    $ git submodule update --init
    $ cd java

Compile package:

.. code-block:: bash

    $ mvn clean install -DskipTests

This will build GraphAr C++ library internally for Java. If you already installed GraphAr C++ library in your system,
you can append this option to skip: ``-DbuildGarCPP=OFF``.

Then set GraphAr as a dependency in maven project:

.. code-block:: xml

   <dependencies>
       <dependency>
         <groupId>org.apache.graphar</groupId>
         <artifactId>gar-java</artifactId>
         <version>0.1.0</version>
       </dependency>
   </dependencies>

How to use
----------

Information classes
~~~~~~~~~~~~~~~~~~~

The Java library for GraphAr provides distinct information classes for
constructing and accessing meta information about graphs, vertices, and
edges. These classes act as essential parameters for constructing
readers and writers, and they can be built either from the existing meta
files (in the Yaml format) or in-memory from scratch.

To construct information from a Yaml file, please refer to the following
example code.

.. code-block:: java

   // read graph yaml and construct information
   String path = ...; // the path to the yaml file
   Result<GraphInfo> graphInfoResult = GraphInfo.load(path);
   if (!graphInfoResult.hasError()) {
       GraphInfo graphInfo = graphInfoResult.value();
       // use information classes
       StdMap<StdString, VertexInfo> vertexInfos = graphInfo.getVertexInfos();
       StdMap<StdString, EdgeInfo> edgeInfos = graphInfo.getEdgeInfos();
   }

See `test for
graphinfo <https://github.com/alibaba/GraphAr/tree/main/java/src/test/java/com/alibaba/graphar/graphinfo>`__
for the complete example.

Writers
~~~~~~~

The GraphAr Java writers wrap C++ interfaces to write arrow::Table into GraphAr
formatted files in a batch-import fashion. But arrow::Table is not easy
to build in Java. Instead, the GraphAr Java library provide a static
method to convert VectorSchemaRoot into arrow::Table. Warning: There are
some problems concerning this method which lead to memory leaks. We will
fix it or rewrite writers with Apache arrow Java.

With the VertexWriter, users can specify a particular property group to
be written into its corresponding chunks, or choose to write all
property groups. For edge chunks, besides the meta data (edge info), the
adjList type should also be specified. The adjList/properties can be
written alone, or alternatively, all adjList, properties, and the offset
(for CSR and CSC format) chunks can be written simultaneously.

To utilize the GAR Java writer, please refer to the following example
code.

.. code-block:: java

   // common steps to construct VectorSchemaRoot
   String uri = "file:" + ...; // data source
   ScanOptions options = new ScanOptions(/*batchSize*/ 32768);
   StdSharedPtr<ArrowTable> table = null;
   try (BufferAllocator allocator = new RootAllocator();
           DatasetFactory datasetFactory =
              new FileSystemDatasetFactory(
                      allocator, NativeMemoryPool.getDefault(), FileFormat.PARQUET, uri);
           Dataset dataset = datasetFactory.finish();
           Scanner scanner = dataset.newScan(options);
           ArrowReader reader = scanner.scanBatches()) {
       while (reader.loadNextBatch()) {
           try (VectorSchemaRoot root = reader.getVectorSchemaRoot()) {
               // convert VectorSchemaRoot to ArrowTable
               table = ArrowTable.fromVectorSchemaRoot(allocator, root, reader);
           }
       }
   } catch (Exception e) {
               e.printStackTrace();
   }

   // construct writer object
   String path = ...; // file to be wrote
   StdString edgeMetaFile = StdString.create(path);
   StdSharedPtr<Yaml> edgeMeta = Yaml.loadFile(edgeMetaFile).value();
   EdgeInfo edgeInfo = EdgeInfo.load(edgeMeta).value();
   EdgeChunkWriter writer = EdgeChunkWriter.factory.create(
                           edgeInfo, StdString.create("/tmp/"), AdjListType.ordered_by_source);

   // write table with writer object
   writer.sortAndWriteAdjListTable(table, 0, 0); // Write adj list of vertex chunk 0 to files

See `test for
writers <https://github.com/alibaba/GraphAr/tree/main/java/src/test/java/com/alibaba/graphar/writers>`__
for the complete example.

Readers
~~~~~~~

The GraphAr Java reader provides an extensive set of interfaces to read
GAR files. It reads a collection of vertices or edges at a time as
ArrowTable. Similar with the reader in C++ library, it supports the
users to specify the data they need, e.g., a single property group.

To utilize the GAR Java reader, please refer to the following example
code.

.. code-block:: java

   // construct vertex chunk reader
   GraphInfo graphInfo = ...; // load graph meta info
   StdString label = StdString.create("person");
   StdString propertyName = StdString.create("id");
   if (graphInfo.getVertexInfo(label).hasError()) {
       // throw Exception or do other things
   }
   PropertyGroup group = graphInfo.getVertexPropertyGroup(label, propertyName).value();
   Result<VertexPropertyArrowChunkReader> maybeReader =
                   GrapharStaticFunctions.INSTANCE.constructVertexPropertyArrowChunkReader(
                           graphInfo, label, group);
   // check reader's status if needed
   VertexPropertyArrowChunkReader reader = maybeReader.value();
   Result<StdSharedPtr<ArrowTable>> result = reader.getChunk();
   // check table's status if needed
   StdSharedPtr<ArrowTable> table = result.value();
   StdPair<Long, Long> range = reader.getRange().value();

See `test for
readers <https://github.com/alibaba/GraphAr/tree/main/java/src/test/java/com/alibaba/graphar/readers>`__
for the complete example.

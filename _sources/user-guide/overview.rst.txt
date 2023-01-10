Overview
============================

What is GraphAr
------------------------

Graph processing serves as the essential building block for a diverse variety of real-world applications such as social network analytics, data mining, network routing, and scientific computing. As the graph processing becomes increasingly important, there are many in-memory and out-of-core graph storages, databases, graph computing systems and interactive graph query frameworks have emerged.

To accommodate this fragmented graph processing ecology, **GraphAr (Graph Archive, GAR)** is established to enable diverse graph applications or existing systems to build and access the graph data conveniently and efficiently. It specifies a standardized system-independent file format for graphs and provides a set of libraries to generate, access and transform such formatted files.

GraphAr is intended to serve as the standard file format for importing/exporting and persistent storage of the graph data which can be used by diverse existing systems, reducing the overhead when various systems co-work. Additionally, it can also serve as the direct data source for graph processing applications.

The GraphAr project includes such topics as:

- Design of the Graph Archive (GAR) file format. (see `GraphAr File Format <file-format.html>`_)
- A set of libraries for reading, writing and transforming GAR files. (now `the C++ library <../reference/api-reference-cpp.html>`_ and `the Spark library <spark-lib.html>`_ are available)
- Examples about how to use GraphAr to write graph algorithms, or to work with existing systems such as GraphScope. (see `Application Cases <../applications/out-of-core.html>`_)

.. image:: ../images/overview.png
   :alt: overview


GraphAr Features
------------------------

The features of GraphAr include:

- It supports the property graphs and different representations for the graph structure (COO, CSR and CSC).
- It is compatible with existing widely-used file types including CSV, ORC and Parquet.
- Apache Spark can be utilized to generate, load and transform the GAR files.
- It is convenient to be used by a variety of single-machine/distributed graph processing systems, databases, and other downstream computing tasks.
- It enables to modify the topology structure or the properties of the graph, or to construct a new graph with a set of selected vertices/edges.
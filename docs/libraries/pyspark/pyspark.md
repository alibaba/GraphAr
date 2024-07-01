---
id: pyspark
title: PySpark Library
sidebar_position: 4
---


:::note

The current policy of GraphAr project is that for Apache Spark
the main API is Scala Spark API. PySpark API follows scala Spark API.
Please refer to [GraphAr Spark library](../spark/spark.md)
for more detailed information about how to use GraphAr with Apache
Spark.

:::

## Overview

The GraphAr PySpark library is provided for generating, loading and
transforming GraphAr format files with PySpark.

- **Information Classes**: As same with in the C++ library, the
  information classes are implemented as a part of the PySpark library
  for constructing and accessing the meta information about the graphs,
  vertices and edges in GraphAr.
- **IndexGenerator**: The IndexGenerator helps to generate the indices
  for vertex/edge DataFrames. In most cases, IndexGenerator is first
  utilized to generate the indices for a DataFrame (e.g., from primary
  keys), and then this DataFrame can be written into GraphAr format files through
  the writer.
- **Writer**: The GraphAr PySpark writer provides a set of interfaces
  that can be used to write Spark DataFrames into GraphAr format files. Every time
  it takes a DataFrame as the logical table for a type of vertices or
  edges, assembles the data in specified format (e.g., reorganize the
  edges in the CSR way) and then dumps it to standard GraphAr format files (CSV,
  ORC or Parquet files) under the specific directory path.
- **Reader**: The GraphAr PySpark reader provides a set of interfaces
  that can be used to read GraphAr format files. It reads a collection of vertices
  or edges at a time and assembles the result into the Spark DataFrame.
  Similar with the reader in the C++ library, it supports the users to
  specify the data they need, e.g., reading a single property group
  instead of all properties.

## Use Cases

The GraphAr Spark library can be used in a range of scenarios:

- Taking GraphAr format data as a data source to execute SQL queries or do graph
  processing (e.g., using GraphX).
- Transforming data between GraphAr format data and other data sources (e.g., Hive,
  Neo4j, NebulaGraph, …).
- Transforming GraphAr format data between different file types (e.g., from ORC to
  Parquet).
- Transforming GraphAr format data between different adjList types (e.g., from COO
  to CSR).
- Modifying existing GraphAr format data (e.g., adding new vertices/edges).

## Get GraphAr Spark Library

### Building from source

GraphAr PySpark uses poetry as a build system. Please refer to
[Poetry documentation](https://python-poetry.org/docs/#installation)
to find the manual how to install this tool. Currently GraphAr PySpark
is build with Python 3.9 and PySpark 3.2

Make the graphar-pyspark-library directory as the current working
directory:

```bash
cd incubator-graphar/pyspark
```

Build package:

```bash
poetry build
```

After compilation, a similar file *graphar_pyspark-0.0.1.tar.gz* is
generated in the directory *pyspark/dist/*.

### Get from PyPI

You cannot install graphar-pyspark from PyPi for now.

## How to Use

### Initialization

GraphAr PySpark is not a standalone library but bindings to GraphAr
Scala. You need to have *spark-x.x.x.jar* in your *spark-jars*.
Please refer to [GraphAr scala documentation](../spark/spark.md) to get
this JAR.

```python
// create a SparkSession from pyspark.sql import SparkSession

spark = ( SparkSession .builder. … .conf(“spark-jars”,
“path-to-graphar-spark-x.x.x.jar-file”) .getOrCreate() )

from graphar_pyspark import initialize initialize(spark)
```

After initialization you can use the same API like in GraphAr scala
library.

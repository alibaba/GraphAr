---
title: Implementation Status
sidebar_position: 2
---

The following tables summarize the features available in the various official GraphAr libraries.
All libraries currently follow version 1.0.0 of the GraphAr format.  

## Data Types

| Data type (primitive) | C++ | Java | Scala | Python |
| --------------------- | --- | ---- | ----- | ------ |
| Boolean               | ✓   | ✓    | ✓     | ✓      |
| Int32                 | ✓   | ✓    | ✓     | ✓      |
| Int64                 | ✓   | ✓    | ✓     | ✓      |
| Float                 | ✓   | ✓    | ✓     | ✓      |
| Double                | ✓   | ✓    | ✓     | ✓      |
| String                | ✓   | ✓    | ✓     | ✓      |
| Date                  | ✓   |      |       |        |
| Timestamp             | ✓   |      |       |        |
| Time                  |     |      |       |        |

| Data type (nested)    | C++ | Java | Scala | Python |
| --------------------- | --- | ---- | ----- | ------ |
| List (*)              | ✓   |      |       |        |

:::note

- \(\*) The data type of List is not supported by the CSV payload file format.

:::

## Payload Data File Formats

| Format                      | C++     | Java    | Scala | Python     |
|-----------------------------|---------|---------|-------|------------|
| CSV                         | R/W     | R (1)   | R/W   | R/W (2)    |
| ORC                         | R/W     | R (1)   | R/W   | R/W (2)    |
| Parquet                     | R/W     | R (1)   | R/W   | R/W (2)    |
| JSON                        | R       |         | R/W   | R/W (2)    |
| Avro                        |         |         |       |            |
| HDF5                        |         |         |       |            |

:::note

- *R* - Read supported
- *W* - Write supported

:::

Supported compression methods for the file formats:

| Compression                 | C++     | Java    | Scala | Python     |
|-----------------------------|---------|---------|-------|------------|
| ZSTD (*)                    | ✓       | ✓       | ✓     | ✓          |

:::note

- \(\*) Compression is not supported by the CSV payload file format.

:::

## Property

| Property feature  | C++   | Java  | Scala |   Python   |
|-------------------|-------|-------|-------|------------|
| primary key       | ✓     | ✓     | ✓     | ✓          |
| nullable          | ✓     |       | ✓     | ✓          |

Supported operations in Property:

| Property operation| C++   | Java  | Scala |   Python   |
|-------------------|-------|-------|-------|------------|
| create            | ✓     | ✓ (1) | ✓     | ✓ (2)      |
| get_name          | ✓     | ✓ (1) | ✓     | ✓ (2)      |
| is_primary_key    | ✓     | ✓ (1) | ✓     | ✓ (2)      |
| is_nullable       | ✓     |       | ✓     | ✓ (2)      |

## Property Group

| Property Group (operation) | C++   |Java (1)| Scala |  Python (2)|
|-------------------|-------|--------|-------|------------|
| create            | ✓     | ✓      | ✓     | ✓          |
| add property      | ✓     | ✓      | ✓     | ✓          |
| remove property   |       |        |       |            |
| get properties    | ✓     | ✓      | ✓     | ✓          |
| check property    | ✓     | ✓      |       |            |
| get file type     | ✓     | ✓      | ✓     | ✓          |
| get path prefix   | ✓     | ✓      | ✓     | ✓          |
| check validation  | ✓     |        |       |            |

## Adjacency List

| Adjacency List (type) | C++   | Java  | Scala |   Python   |
|-------------------|-------|-------|-------|------------|
| CSR               | ✓     | ✓     | ✓     | ✓          |
| CSC               | ✓     | ✓     | ✓     | ✓          |
| COO               | ✓     | ✓     | ✓     | ✓          |

Supported operations in Adjacency List:

| Adjacency List (operation) | C++   |Java (1)| Scala |  Python (2)|
|-------------------|-------|--------|-------|------------|
| create            | ✓     |        | ✓     | ✓          |
| get adjacency type| ✓     |        | ✓     | ✓          |
| get file type     | ✓     |        | ✓     | ✓          |
| get path prefix   | ✓     |        | ✓     | ✓          |
| check validation  | ✓     |        |       |            |

## Vertex

Vertex features:

| Vertex feature    | C++   | Java  | Scala |   Python   |
|-------------------|-------|-------|-------|------------|
| label             | ✓     | ✓     | ✓     | ✓          |
| tag               |       |       |       |            |
| chunk based       | ✓     | ✓     | ✓     | ✓          |
| property group    | ✓     | ✓     | ✓     | ✓          |

:::note

- *label* is the vertex label, which is a unique identifier for the vertex.
- *tag* is the vertex tag, which is tag or category for the vertex.

:::

Supported operations in Vertex Info:

| Vertex Info (operation) | C++   |Java (1)| Scala | Python (2) |
|-------------------|-------|--------|-------|------------|
| create            | ✓     | ✓      | ✓     | ✓          |
| add group         | ✓     | ✓      | ✓     | ✓          |
| remove group      |       |        |       |            |
| get label         | ✓     | ✓      | ✓     | ✓          |
| get chunk size    | ✓     | ✓      | ✓     | ✓          |
| get groups        | ✓     | ✓      | ✓     | ✓          |
| get path prefix   | ✓     | ✓      | ✓     | ✓          |
| check property    | ✓     | ✓      | ✓     | ✓          |
| check validation  | ✓     |        | ✓     | ✓          |
| serialize         | ✓     | ✓      | ✓     | ✓          |
| deserialize       | ✓     | ✓      | ✓     | ✓          |

## Edge

Edge features:

| Edge feature      | C++   | Java  | Scala |   Python   |
|-------------------|-------|-------|-------|------------|
| label             | ✓     | ✓     | ✓     | ✓          |
| chunk based       | ✓     | ✓     | ✓     | ✓          |
| property group    | ✓     | ✓     | ✓     | ✓          |
| adjacent list     | ✓     | ✓     | ✓     | ✓          |
| directed          | ✓     | ✓     | ✓     | ✓          |

Supported operations in Edge Info:

| Edge Info (operation) | C++   |Java (1)| Scala | Python (2) |
|-------------------|-------|--------|-------|------------|
| create            | ✓     | ✓      | ✓     | ✓          |
| add group         | ✓     | ✓      | ✓     | ✓          |
| remove group      |       |        |       |            |
| add adj list      | ✓     | ✓      | ✓     | ✓          |
| remove adj list   |       |        |       |            |
| get label         | ✓     | ✓      | ✓     | ✓          |
| get source label  | ✓     | ✓      | ✓     | ✓          |
| get dest label    | ✓     | ✓      | ✓     | ✓          |
| get chunk size    | ✓     | ✓      | ✓     | ✓          |
| get source chunk size  | ✓     | ✓      | ✓     | ✓          |
| get dest chunk size   | ✓     | ✓      | ✓     | ✓          |
| get groups        | ✓     | ✓      | ✓     | ✓          |
| check adj list    | ✓     | ✓      | ✓     | ✓          |
| check property    | ✓     | ✓      | ✓     | ✓          |
| get file type     | ✓     | ✓      | ✓     | ✓          |
| get path prefix   | ✓     | ✓      | ✓     | ✓          |
| is directed       | ✓     | ✓      | ✓     | ✓          |
| check validation  | ✓     |        | ✓     | ✓          |
| serialize         | ✓     | ✓      | ✓     | ✓          |
| deserialize       | ✓     | ✓      | ✓     | ✓          |

:::note

- *\<source label, label, dest label\>* is the unique identifier for the edge type.

:::

## Graph

| Graph             | C++   | Java  | Scala |   Python   |
|-------------------|-------|-------|-------|------------|
| labeled vertex (with property)    | ✓     | ✓     | ✓     | ✓          |
| labeled edge (with property)     | ✓     | ✓     | ✓     | ✓          |
| extra info        | ✓     |       |       |            |

Supported operations in Graph Info:

| Graph Info (operation) | C++   |Java (1)| Scala | Python (2) |
|-------------------|-------|--------|-------|------------|
| create            | ✓     | ✓      | ✓     | ✓          |
| add vertex        | ✓     | ✓      | ✓     | ✓          |
| remove vertex     |       |        |       |            |
| add edge          | ✓     | ✓      | ✓     | ✓          |
| remove edge       |       |        |       |            |
| get name          | ✓     | ✓      | ✓     | ✓          |
| get vertex        | ✓     | ✓      | ✓     | ✓          |
| get edge          | ✓     | ✓      | ✓     | ✓          |
| add extra info    |       |        |       |            |
| remove extra info |       |        |       |            |
| get extra info    | ✓     |        |       |            |
| check validation  | ✓     |        |       |            |
| serialize         | ✓     | ✓      | ✓     | ✓          |
| deserialize       | ✓     | ✓      | ✓     | ✓          |

:::note

- \(1) Through fastFFI bindings to the GraphAr C++ library.

- \(2) Through py4j bindings to the GraphAr Spark library.

:::

## Libraries Version Compatibility

| GraphAr C++ Version | C++ | CMake | Format Version |
|---------------------|-----|-------|----------------|
| 0.11.x              | 17+ | 2.8+  | 1.0.0          |

| GraphAr Java Version | Java | Maven | Format Version |
|----------------------|------|-------|----------------|
| 0.1.0                | 1.8  | 3.6+  | 1.0.0          |

| GraphAr Spark Version | Apache Spark Version | Scala Version | Java Version | Hadoop Version | Format Version |
|-----------------------|----------------------|---------------|--------------|----------------|----------------|
| 0.1.0                 | 3.2.x-3.3.x          | 2.12.x        | 1.8, 11      | 3              | 1.0.0          |

| GraphAr PySpark Version | Python Version | PySpark Version | Hadoop Version | Format Version |
|-------------------------|----------------|-----------------|----------------|----------------|
| 0.1.0                   | 3.8+           | 3.2.x           | 3              | 1.0.0          |

:::note

- Since the GraphAr PySpark library is bindings to the GraphAr Spark library,
  the PySpark version should be compatible with the Spark version.

:::

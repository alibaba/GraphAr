---
id: concepts
title: Concepts
sidebar_position: 3
---

Glossary of relevant concepts and terms.

- **Property Group**: GraphAr splits the properties of vertex/edge into groups to allow for efficient storage
  and access without the need to load all properties. Also benefits appending of new properties. Each property
  group is the unit of storage and is stored in a separate directory.

- **Adjacency List**: The storage method to store the edges of certain vertex type. Which include:
  - *ordered by source vertex id*: the edges are ordered and aligned by the source vertex
  - *ordered by destination vertex id*: the edges are ordered and aligned by the destination vertex
  - *unordered by source vertex id*: the edges are unordered but aligned by the source vertex
  - *unordered by destination vertex id*: the edges are unordered but aligned by the destination vertex

- **Compressed Sparse Row (CSR)**: The storage layout the edges of certain vertex type. Corresponding to the
  ordered by source vertex id adjacency list, the edges are stored in a single array and the offsets of the
  edges of each vertex are stored in a separate array.

- **Compressed Sparse Column (CSC)**: The storage layout the edges of certain vertex type. Corresponding to the
  ordered by destination vertex id adjacency list, the edges are stored in a single array and the offsets of the
  edges of each vertex are stored in a separate array.

- **Coordinate List (COO)**: The storage layout the edges of certain vertex type. Corresponding to the unordered
  by source vertex id or unordered by target vertex id adjacency list, the edges are stored in a single array and
  no offsets are stored.

- **Vertex Chunk**: The storage unit of vertex. Each vertex chunk contains a fixed number of vertices and is stored
  in a separate file.

- **Edge Chunk**: The storage unit of edge. Each edge chunk contains a fixed number of edges and is stored in a separate file.

**Highlights**:
  The design of property group and vertex/edge chunk allows users to
    - Access the data without reading all the data into memory
    - Conveniently append new properties to the graph without the need to reorganize the data
    - Efficiently store and access the data in a distributed environment and parallel processing

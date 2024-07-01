---
id: motivation
title: Motivation
sidebar_position: 2
---

Numerous graph systems,
such as Neo4j, Nebula Graph, and Apache HugeGraph, have been developed in recent years.
Each of these systems has its own graph data storage format, complicating the exchange of graph data between different systems.
The need for a standard data file format for large-scale graph data storage and processing that can be used by diverse existing systems is evident, as it would reduce overhead when various systems work together.

Our aim is to fill this gap and contribute to the open-source community by providing a standard data file format for graph data storage and exchange, as well as for out-of-core querying.
This format, which we have named GraphAr, is engineered to be efficient, cross-language compatible, and to support out-of-core processing scenarios, such as those commonly found in data lakes.
Furthermore, GraphAr's flexible design ensures that it can be easily extended to accommodate a broader array of graph data storage and exchange use cases in the future.

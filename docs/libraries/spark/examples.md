---
id: examples
title: Examples
sidebar_position: 1
---


## Co-Work with Apache Spark

[Apache Spark](https://spark.apache.org/) is a multi-language engine for executing data engineering, data science, and machine learning on single-node machines or clusters. The GraphAr Spark library is developed to make the integration of GraphAr with Spark easy. This library allows users to efficiently generate, load, and transform GraphAr format files, and to integrate GraphAr with other Spark-compatible systems.

Examples of this co-working integration have been provided as showcases.

### Examples

### Transform GraphAr format files

We provide an example in [TestGraphTransformer.scala][test-graph-transformer], which demonstrates
how to conduct data transformation at the graph level. [TransformExample.scala][transformer-example] is
another example for graph data conversion between different file types or different
adjList types, which is implemented at the vertex/edge table level. To do this,
the original data is first loaded into a Spark DataFrame using the GraphAr Spark Reader.
Then, the DataFrame is written into generated GraphAr format files through a GraphAr Spark Writer,
following the meta data defined in a new information file.

### Compute with GraphX

Another important use case of GraphAr is to use it as a data source for graph
computing or analytics; [ComputeExample.scala][compute-example] provides an example of constructing
a GraphX graph from reading GraphAr format files and executing a connected-components computation.
Also, executing queries with Spark SQL and running other graph analytic algorithms
can be implemented in a similar fashion.

### Import/Export graphs of Neo4j

[Neo4j](https://neo4j.com/product/neo4j-graph-database) graph database provides
a [spark connector](https://neo4j.com/docs/spark/current/overview/) for integration
with Spark. The Neo4j Spark Connector, combined with the GraphAr Spark library,
enables us to migrate graph data between Neo4j and GraphAr. This is also a key application
of GraphAr in which it acts as a persistent storage of graph data in the database.

We provide two example programs that demonstrate how GraphAr can be used in conjunction
with Neo4j. It utilizes one of the built-in Neo4j datasets, the [Movie Graph](https://neo4j.com/developer/example-data/#built-in-examples),
which is a mini graph application containing actors and directors that are related through the movies they have collaborated on.
Given some necessary information like the chunk size, the prefix of the file path, and the file type,
the program can read the graph data from Neo4j and write it into GraphAr files.
When exporting graph data from Neo4j and writing to GraphAr, please refer to the following code,
with [Neo4j2GraphAr.scala][neo4j2graphar] providing a complete example.

```scala
def main(args: Array[String]): Unit = {
    // connect to the Neo4j instance
    val spark = SparkSession.builder()
      .appName("Neo4j to GraphAr for Movie Graph")
      .config("neo4j.url", "bolt://localhost:7687")
      .config("neo4j.authentication.type", "basic")
      .config("neo4j.authentication.basic.username", sys.env.get("Neo4j_USR").get)
      .config("neo4j.authentication.basic.password", sys.env.get("Neo4j_PWD").get)
      .config("spark.master", "local")
      .getOrCreate()

    // initialize a graph writer
    val writer: GraphWriter = new GraphWriter()

    // put movie graph data into writer
    readAndPutDataIntoWriter(writer, spark)

    // write in GraphAr format
    val outputPath: String = args(0)
    val vertexChunkSize: Long = args(1).toLong
    val edgeChunkSize: Long = args(2).toLong
    val fileType: String = args(3)

    writer.write(outputPath, spark, "MovieGraph", vertexChunkSize, edgeChunkSize, fileType)
}
```

The `readAndPutDataIntoWriter` method read the vertex and edge from Neo4j to DataFrame
(Refer to [Neo4j docs](https://neo4j.com/docs/spark/current/reading/) for more details),
and then put then into a GraphAr GraphWriter.

```scala
def readAndPutDataIntoWriter(writer: GraphWriter, spark: SparkSession): Unit = {
  // read vertices with label "Person" from Neo4j as a DataFrame
  val person_df = spark.read.format("org.neo4j.spark.DataSource")
    .option("query", "MATCH (n:Person) RETURN n.name AS name, n.born as born")
    .load()
  // put into writer, vertex label is "Person"
  writer.PutVertexData("Person", person_df)

  // read vertices with label "Movie" from Neo4j as a DataFrame
  val movie_df = spark.read.format("org.neo4j.spark.DataSource")
    .option("query", "MATCH (n:Movie) RETURN n.title AS title, n.tagline as tagline")
    .load()
  // put into writer, vertex label is "Movie"
  writer.PutVertexData("Movie", movie_df)

  // read edges with type "Person"->"PRODUCED"->"Movie" from Neo4j as a DataFrame
  val produced_edge_df = spark.read.format("org.neo4j.spark.DataSource")
    .option("query", "MATCH (a:Person)-[r:PRODUCED]->(b:Movie) return a.name as src, b.title as dst")
    .load()
  // put into writer, source vertex label is "Person", edge label is "PRODUCED"
  // target vertex label is "Movie"
  writer.PutEdgeData(("Person", "PRODUCED", "Movie"), produced_edge_df)
```

Finally, the `write` method writes the graph data in GraphAr format to the specified path.

Additionally, when importing data from GraphAr to create/update instances in Neo4j, please refer to the following code:

```scala
def main(args: Array[String]): Unit = {
    // connect to the Neo4j instance
    val spark = SparkSession.builder()
      .appName("GraphAr to Neo4j for Movie Graph")
      .config("neo4j.url", "bolt://localhost:7687")
      .config("neo4j.authentication.type", "basic")
      .config("neo4j.authentication.basic.username", sys.env.get("Neo4j_USR").get)
      .config("neo4j.authentication.basic.password", sys.env.get("Neo4j_PWD").get)
      .config("spark.master", "local")
      .getOrCreate()

    // path to the graph information file
    val graphInfoPath: String = args(0)
    val graphInfo = GraphInfo.loadGraphInfo(graphInfoPath, spark)

    val graphData = GraphReader.read(graphInfoPath, spark)
    val vertexData = graphData._1
    val edgeData = graphData._2

    putVertexDataIntoNeo4j(graphInfo, vertexData, spark)
    putEdgeDataIntoNeo4j(graphInfo, vertexData, edgeData, spark)
}
```

Pass the graph information file path to `loadGraphInfo` method to get the graph information.
Then, read the graph data from GraphAr files with `GraphReader` as DataFrame pair,
`_1` for vertices and `_2` for edges.

The `putVertexDataIntoNeo4j` and `putEdgeDataIntoNeo4j` methods creates or updates the vertices DataFrame and edges DataFrame in Neo4j.

```scala
def putVertexDataIntoNeo4j(graphInfo: GraphInfo,
                           vertexData: Map[String, DataFrame],
                           spark: SparkSession): Unit = {
  // write each vertex type to Neo4j
  vertexData.foreach { case (key, df) => {
    val primaryKey = graphInfo.getVertexInfo(key).getPrimaryKey()
    // the vertex index column is not needed in Neo4j
    // write to Neo4j, refer to https://neo4j.com/docs/spark/current/writing/
    df.drop(GeneralParams.vertexIndexCol).write.format("org.neo4j.spark.DataSource")
      .mode(SaveMode.Overwrite)
      .option("labels", ":" + key)
      .option("node.keys", primaryKey)
      .save()
  }}
}

def putEdgeDataIntoNeo4j(graphInfo: GraphInfo,
                         vertexData: Map[String, DataFrame],
                         edgeData: Map[(String, String, String), Map[String, DataFrame]],
                         spark: SparkSession): Unit = {
  // write each edge type to Neo4j
  edgeData.foreach { case (key, value) => {
    // key is (source vertex label, edge label, target vertex label)
    val sourceLabel = key._1
    val edgeLabel = key._2
    val targetLabel = key._3
    val sourcePrimaryKey = graphInfo.getVertexInfo(sourceLabel).getPrimaryKey()
    val targetPrimaryKey = graphInfo.getVertexInfo(targetLabel).getPrimaryKey()
    val sourceDf = vertexData(sourceLabel)
    val targetDf = vertexData(targetLabel)
    // convert the source and target index column to the primary key column
    val df = Utils.joinEdgesWithVertexPrimaryKey(value.head._2, sourceDf, targetDf, sourcePrimaryKey, targetPrimaryKey)  // use the first DataFrame of (adj_list_type_str, DataFrame) map

    val properties = if (edgeLabel == "REVIEWED") "rating,summary" else ""

    // write to Neo4j, refer to https://neo4j.com/docs/spark/current/writing/
    df.write.format("org.neo4j.spark.DataSource")
      .mode(SaveMode.Overwrite)
      .option("relationship", edgeLabel)
      .option("relationship.save.strategy", "keys")
      .option("relationship.source.labels", ":" + sourceLabel)
      .option("relationship.source.save.mode", "match")
      .option("relationship.source.node.keys", "src:" + sourcePrimaryKey)
      .option("relationship.target.labels", ":" + targetLabel)
      .option("relationship.target.save.mode", "match")
      .option("relationship.target.node.keys", "dst:" + targetPrimaryKey)
      .option("relationship.properties", properties)
      .save()
  }}
}
```

Finally, you will see the graph in Neo4j Browser after running the above code.

See [GraphAr2Neo4j.scala][graphar2neo4j] for the complete example.

:::tip

- The Neo4j Spark Connector offers different save modes and writing options, such as Append(CREATE) or Overwrite(MERGE). Please refer to its [documentation](https://neo4j.com/docs/spark/current/writing/) for more information and take the most appropriate method while using.
- The Neo4j Spark Connector supports to use [Spark structured streaming API](https://neo4j.com/docs/spark/current/streaming), which works differently from Spark batching. One can utilize this API to read/write a stream from/to Neo4j, avoiding to maintain all data in the memory.

:::

[test-graph-transformer]: https://github.com/apache/incubator-graphar/blob/main/maven-projects/spark/graphar/src/test/scala/org/apache/graphar/TestGraphTransformer.scala
[transformer-example]: https://github.com/apache/incubator-graphar/blob/main/maven-projects/spark/graphar/src/test/scala/org/apache/graphar/TransformExample.scala
[compute-example]: https://github.com/apache/incubator-graphar/blob/main/maven-projects/spark/graphar/src/test/scala/org/apache/graphar/ComputeExample.scala
[neo4j2graphar]: https://github.com/apache/incubator-graphar/blob/main/maven-projects/spark/graphar/src/main/scala/org/apache/graphar/example/Neo4j2GraphAr.scala
[graphar2neo4j]: https://github.com/apache/incubator-graphar/blob/main/maven-projects/spark/graphar/src/main/scala/org/apache/graphar/example/GraphAr2Neo4j.scala

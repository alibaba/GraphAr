/** Copyright 2022 Alibaba Group Holding Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.alibaba.graphar.graph

import com.alibaba.graphar.{GeneralParams, AdjListType, GraphInfo, VertexInfo, EdgeInfo}
import com.alibaba.graphar.reader.{VertexReader, EdgeReader}

import org.apache.spark.sql.{DataFrame, SparkSession}
import org.apache.spark.sql.types._
import org.apache.spark.sql.functions._

/** The helper object for read graphs through the definitions of their infos. */
object GraphReader {
  private def readAllVertices(prefix: String, vertexInfos: Map[String, VertexInfo], spark: SparkSession): Map[String, DataFrame] = {
    val vertex_dataframes: Map[String, DataFrame] = vertexInfos.MapValues { vertexInfo => {
      val reader = new VertexReader(prefix, vertexInfo, spark)
      (vertex_info.getLabel, reader.readAllVertexPropertyGroups(true))
    }}
    return vertex_dataframes
  }

  private def readAllEdges(prefix: String, edgeInfos: Map[String, edgeInfo], spark: SparkSession): Map[String, Map[AdjListType, DataFrame]] = {
    val edge_dataframes: Map[String, Map[AdjListType, DataFrame]] = edgeInfos.MapValues { edgeInfo => {
      val adj_lists = edgeInfo.getAdj_lists
      val adj_list_it = adj_lists.iterator
      var adj_list_type_edge_df_map: Map[AdjListType, DataFrame] = Map[AdjListType, DataFrame]()
      while (adj_list_it.hasNext()) {
        val adj_list_type = adj_list_it.next().getAdjList_type_in_gar
        val reader = new EdgeReader(prefix, edgeInfo, adj_list_type, spark)
        adj_list_type_edge_df_map += (adj_list_type, reader.readEdges(false))
      }
      (edgeInfo.getConcatKey(), adj_list_type_edge_df_map)
    }}
    return edge_dataframes
  }

  def read(graphInfo: GraphInfo, spark: SparkSession): Pair[Map[String, DataFrame], Map[String, Map[AdjListType, DataFrame]]] = {
    val prefix = graphInfo.getPrefix
    val vertex_infos = graphInfo.getVertexInfos()
    val edge_infos = graphInfo.getEdgeInfos()
    return (readAllVertices(prefix, vertexInfos, spark), readAllEdges(prefix, edgeInfos, spark))
  }

  def read(graphInfoPath: String, spark: SparkSession): Pair[Map[String, DataFrame], Map[String, Map[AdjListType, DataFrame]]] = {
    // load graph info
    val graph_info = GraphInfo.loadGraphInfo(graphInfoPath, spark)

    // conduct reading
    read(graph_info, spark)
  }
}

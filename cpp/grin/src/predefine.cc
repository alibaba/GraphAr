/** Copyright 2022 Alibaba Group Holding Limited.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <algorithm>
#include <string>
#include <vector>

#include "grin/src/predefine.h"

std::string GetDataTypeName(GRIN_DATATYPE type) {
  switch (type) {
  case GRIN_DATATYPE::Int32:
    return "int32";
  case GRIN_DATATYPE::UInt32:
    return "uint32";
  case GRIN_DATATYPE::Int64:
    return "int64";
  case GRIN_DATATYPE::UInt64:
    return "uint64";
  case GRIN_DATATYPE::Float:
    return "float";
  case GRIN_DATATYPE::Double:
    return "double";
  case GRIN_DATATYPE::String:
    return "string";
  case GRIN_DATATYPE::Date32:
    return "date32";
  case GRIN_DATATYPE::Time32:
    return "time32";
  case GRIN_DATATYPE::Timestamp64:
    return "timestamp64";
  default:
    return "undefined";
  }
}

GRIN_DATATYPE GARToDataType(GAR_NAMESPACE::DataType type) {
  switch (type.id()) {
  case GAR_NAMESPACE::Type::BOOL:
    return GRIN_DATATYPE::Undefined;
  case GAR_NAMESPACE::Type::INT32:
    return GRIN_DATATYPE::Int32;
  case GAR_NAMESPACE::Type::INT64:
    return GRIN_DATATYPE::Int64;
  case GAR_NAMESPACE::Type::FLOAT:
    return GRIN_DATATYPE::Float;
  case GAR_NAMESPACE::Type::DOUBLE:
    return GRIN_DATATYPE::Double;
  case GAR_NAMESPACE::Type::STRING:
    return GRIN_DATATYPE::String;
  default:
    return GRIN_DATATYPE::Undefined;
  }
}

bool cmp(const GAR_NAMESPACE::EdgeInfo& info1,
         const GAR_NAMESPACE::EdgeInfo& info2) {
  return info1.GetEdgeLabel() < info2.GetEdgeLabel();
}

GRIN_GRAPH_T* get_graph_by_info_path(const std::string& path) {
  // construct graph
  auto maybe_graph_info = GAR_NAMESPACE::GraphInfo::Load(path);
  if (maybe_graph_info.has_error())
    return NULL;
  auto graph_info = maybe_graph_info.value();
  auto graph = new GRIN_GRAPH_T(graph_info);

  // initialize graph
  __grin_init_vertices_collections(graph);
  __grin_init_edges_collections(graph);
  __grin_init_vertex_properties(graph);
  __grin_init_edge_properties(graph);

  return graph;
}

size_t __grin_get_edge_num(GRIN_GRAPH_T* graph, unsigned type_begin,
                           unsigned type_end) {
  size_t res = 0;
  for (auto type_id = type_begin; type_id < type_end; type_id++) {
    if (type_id >= graph->edge_type_num)
      break;
    if (graph->edge_num[type_id] != 0) {
      res += graph->edge_num[type_id];
      continue;
    }

    if (graph->edges_collections[type_id].find(GAR_ORDERED_BY_SOURCE) !=
        graph->edges_collections[type_id].end()) {
      graph->edge_num[type_id] =
          std::get<GAR_NAMESPACE::EdgesCollection<GAR_ORDERED_BY_SOURCE>>(
              graph->edges_collections[type_id].at(GAR_ORDERED_BY_SOURCE))
              .size();
    } else if (graph->edges_collections[type_id].find(GAR_ORDERED_BY_DEST) !=
               graph->edges_collections[type_id].end()) {
      graph->edge_num[type_id] =
          std::get<GAR_NAMESPACE::EdgesCollection<GAR_ORDERED_BY_DEST>>(
              graph->edges_collections[type_id].at(GAR_ORDERED_BY_DEST))
              .size();
    } else if (graph->edges_collections[type_id].find(
                   GAR_UNORDERED_BY_SOURCE) !=
               graph->edges_collections[type_id].end()) {
      graph->edge_num[type_id] =
          std::get<GAR_NAMESPACE::EdgesCollection<GAR_UNORDERED_BY_SOURCE>>(
              graph->edges_collections[type_id].at(GAR_UNORDERED_BY_SOURCE))
              .size();
    } else if (graph->edges_collections[type_id].find(GAR_UNORDERED_BY_DEST) !=
               graph->edges_collections[type_id].end()) {
      graph->edge_num[type_id] =
          std::get<GAR_NAMESPACE::EdgesCollection<GAR_UNORDERED_BY_DEST>>(
              graph->edges_collections[type_id].at(GAR_UNORDERED_BY_DEST))
              .size();
    }
    res += graph->edge_num[type_id];
  }
  return res;
}

void __grin_init_vertices_collections(GRIN_GRAPH_T* graph) {
  graph->vertex_type_num = graph->graph_info.GetVertexInfos().size();
  graph->vertex_types.clear();
  graph->vertex_offsets.clear();
  graph->vertex_offsets.push_back(0);
  graph->tot_vertex_num = 0;
  graph->vertex_chunk_size.clear();

  for (const auto& [label, vertex_info] : graph->graph_info.GetVertexInfos()) {
    auto maybe_vertices_collection =
        GAR_NAMESPACE::ConstructVerticesCollection(graph->graph_info, label);
    auto& vertices = maybe_vertices_collection.value();
    graph->vertex_types.push_back(label);
    graph->tot_vertex_num += vertices.size();
    graph->vertex_offsets.push_back(graph->tot_vertex_num);
    graph->vertices_collections.push_back(std::move(vertices));
    graph->vertex_chunk_size.push_back(vertex_info.GetChunkSize());
  }
}

void __grin_init_edges_collections(GRIN_GRAPH_T* graph) {
  // sort edge infos by edge label
  std::vector<GAR_NAMESPACE::EdgeInfo> all_edge_infos;
  for (const auto& [label, edge_info] : graph->graph_info.GetEdgeInfos()) {
    all_edge_infos.push_back(edge_info);
  }
  std::sort(all_edge_infos.begin(), all_edge_infos.end(), cmp);

  // clear
  graph->unique_edge_type_begin_type.clear();
  graph->unique_edge_type_2_ids.clear();
  graph->unique_edge_types.clear();
  graph->unique_edge_type_num = 0;
  graph->edge_type_num = 0;
  graph->edge_types.clear();
  graph->src_type_ids.clear();
  graph->dst_type_ids.clear();
  graph->unique_edge_type_ids.clear();
  graph->edge_num.clear();
  graph->edges_collections.clear();

  for (const auto& edge_info : all_edge_infos) {
    auto src_label = edge_info.GetSrcLabel();
    auto dst_label = edge_info.GetDstLabel();
    auto edge_label = edge_info.GetEdgeLabel();
    if (graph->unique_edge_type_2_ids.find(edge_label) ==
        graph->unique_edge_type_2_ids.end()) {
      graph->unique_edge_types.push_back(edge_label);
      graph->unique_edge_type_2_ids.insert(
          {edge_label, graph->unique_edge_type_num});
      graph->unique_edge_type_num++;
      graph->unique_edge_type_begin_type.push_back(graph->edge_type_num);
    }

    unsigned src_type_id = std::find(graph->vertex_types.begin(),
                                     graph->vertex_types.end(), src_label) -
                           graph->vertex_types.begin();
    unsigned dst_type_id = std::find(graph->vertex_types.begin(),
                                     graph->vertex_types.end(), dst_label) -
                           graph->vertex_types.begin();
    unsigned unique_edge_type_id = graph->unique_edge_type_2_ids.at(edge_label);
    graph->src_type_ids.push_back(src_type_id);
    graph->dst_type_ids.push_back(dst_type_id);
    graph->unique_edge_type_ids.push_back(unique_edge_type_id);
    graph->edge_types.push_back(edge_label);
    graph->edge_num.push_back(0);
    std::map<GAR_NAMESPACE::AdjListType, GAR_NAMESPACE::Edges> edge_map;
    graph->edges_collections.push_back(edge_map);

    GAR_NAMESPACE::AdjListType adj_list_type = GAR_ORDERED_BY_SOURCE;
    if (edge_info.ContainAdjList(adj_list_type)) {
      auto maybe_edges_collection = GAR_NAMESPACE::ConstructEdgesCollection(
          graph->graph_info, src_label, edge_label, dst_label, adj_list_type);
      auto& edges = maybe_edges_collection.value();
      graph->edges_collections[graph->edge_type_num].insert(
          {adj_list_type, std::move(edges)});
    }

    adj_list_type = GAR_ORDERED_BY_DEST;
    if (edge_info.ContainAdjList(adj_list_type)) {
      auto maybe_edges_collection = GAR_NAMESPACE::ConstructEdgesCollection(
          graph->graph_info, src_label, edge_label, dst_label, adj_list_type);
      auto& edges = maybe_edges_collection.value();
      graph->edges_collections[graph->edge_type_num].insert(
          {adj_list_type, std::move(edges)});
    }

    adj_list_type = GAR_UNORDERED_BY_SOURCE;
    if (edge_info.ContainAdjList(adj_list_type)) {
      auto maybe_edges_collection = GAR_NAMESPACE::ConstructEdgesCollection(
          graph->graph_info, src_label, edge_label, dst_label, adj_list_type);
      auto& edges = maybe_edges_collection.value();
      graph->edges_collections[graph->edge_type_num].insert(
          {adj_list_type, std::move(edges)});
    }

    adj_list_type = GAR_UNORDERED_BY_DEST;
    if (edge_info.ContainAdjList(adj_list_type)) {
      auto maybe_edges_collection = GAR_NAMESPACE::ConstructEdgesCollection(
          graph->graph_info, src_label, edge_label, dst_label, adj_list_type);
      auto& edges = maybe_edges_collection.value();
      graph->edges_collections[graph->edge_type_num].insert(
          {adj_list_type, std::move(edges)});
    }
    graph->edge_type_num++;
  }
  graph->unique_edge_type_begin_type.push_back(graph->edge_type_num);
  graph->tot_edge_num = __grin_get_edge_num(graph, 0, graph->edge_type_num);
}

void __grin_init_vertex_properties(GRIN_GRAPH_T* graph) {
  graph->vertex_properties.clear();
  graph->vertex_property_offsets.clear();
  graph->vertex_property_name_2_ids.clear();
  unsigned property_id = 0, vtype = 0;

  for (const auto& [label, vertex_info] : graph->graph_info.GetVertexInfos()) {
    graph->vertex_property_offsets.push_back(property_id);
    std::map<std::string, unsigned> name_2_id;
    for (auto& group : vertex_info.GetPropertyGroups()) {
      for (auto& property : group.GetProperties()) {
        GRIN_VERTEX_PROPERTY_T vp(vtype, property.name,
                                  GARToDataType(property.type),
                                  property.is_primary);
        graph->vertex_properties.push_back(vp);
        name_2_id.insert({property.name, property_id});
        property_id++;
      }
    }
    graph->vertex_property_name_2_ids.push_back(name_2_id);
    vtype++;
  }
  graph->vertex_property_offsets.push_back(property_id);
}

void __grin_init_edge_properties(GRIN_GRAPH_T* graph) {
  graph->edge_properties.clear();
  graph->edge_property_offsets.clear();
  graph->edge_property_name_2_ids.clear();
  unsigned property_id = 0;

  for (unsigned etype = 0; etype < graph->unique_edge_type_num; etype++) {
    graph->edge_property_offsets.push_back(property_id);
    std::map<std::string, unsigned> name_2_id;
    for (auto et = graph->unique_edge_type_begin_type[etype];
         et < graph->unique_edge_type_begin_type[etype + 1]; ++et) {
      auto& edge_info =
          graph->graph_info
              .GetEdgeInfo(graph->vertex_types[graph->src_type_ids[et]],
                           graph->edge_types[et],
                           graph->vertex_types[graph->dst_type_ids[et]])
              .value();
      auto adj_list_type = graph->edges_collections[et].begin()->first;
      for (auto& group : edge_info.GetPropertyGroups(adj_list_type).value()) {
        for (auto& property : group.GetProperties()) {
          GRIN_EDGE_PROPERTY_T ep(etype, property.name,
                                  GARToDataType(property.type));
          if (name_2_id.find(property.name) != name_2_id.end()) {
            // TODO(lixuecliare): throw exception
            continue;
          }
          graph->edge_properties.push_back(ep);
          name_2_id.insert({property.name, property_id});
          property_id++;
        }
      }
    }
    graph->edge_property_name_2_ids.push_back(name_2_id);
  }
  graph->edge_property_offsets.push_back(property_id);
}

void __grin_init_graph_partitions(GRIN_GRAPH_T* graph, unsigned partition_num,
                                  unsigned partition_id,
                                  GAR_PARTITION_STRATEGY partition_strategy) {
  graph->partition_num = partition_num;
  graph->partition_id = partition_id;
  graph->partition_strategy = partition_strategy;

  // initialize partitioned vertex offsets
  if (partition_strategy == SEGMENTED_PARTITION) {
    graph->partitioned_vertex_offsets.clear();
    for (auto i = 0; i < graph->vertex_type_num; i++) {
      std::vector<size_t> offsets;
      offsets.clear();
      auto vertex_num = graph->vertex_offsets[i + 1] - graph->vertex_offsets[i];
      auto chunk_size = graph->vertex_chunk_size[i];
      auto chunk_num = (vertex_num + chunk_size - 1) / chunk_size;
      auto x = chunk_num / partition_num;
      auto y = chunk_num % partition_num;
      size_t vid = 0;
      for (auto pid = 0; pid < partition_num; pid++) {
        offsets.push_back(std::min(vid, vertex_num));
        if (pid < y) {
          vid += (x + 1) * chunk_size;
        } else {
          vid += x * chunk_size;
        }
      }
      offsets.push_back(std::min(vid, vertex_num));
      graph->partitioned_vertex_offsets.push_back(offsets);
    }
  } else {  // HASH_PARTITION
    graph->partitioned_vertex_offsets.clear();
    for (auto i = 0; i < graph->vertex_type_num; i++) {
      std::vector<size_t> offsets;
      offsets.clear();
      auto vertex_num = graph->vertex_offsets[i + 1] - graph->vertex_offsets[i];
      auto x = vertex_num / partition_num;
      auto y = vertex_num % partition_num;
      size_t vid = 0;
      for (auto pid = 0; pid < partition_num; pid++) {
        offsets.push_back(std::min(vid, vertex_num));
        if (pid < y) {
          vid += (x + 1);
        } else {
          vid += x;
        }
      }
      offsets.push_back(std::min(vid, vertex_num));
      graph->partitioned_vertex_offsets.push_back(offsets);
    }
  }

  // initialize partitioned vertex num
  graph->partitioned_vertex_num.clear();
  for (auto i = 0; i < graph->vertex_type_num; i++) {
    std::vector<size_t> vertex_num;
    vertex_num.clear();
    for (auto pid = 0; pid < partition_num; pid++) {
      vertex_num.push_back(graph->partitioned_vertex_offsets[i][pid + 1] -
                           graph->partitioned_vertex_offsets[i][pid]);
    }
    graph->partitioned_vertex_num.push_back(vertex_num);
  }
}

int64_t __grin_generate_int64_from_id_and_type(GAR_NAMESPACE::IdType id,
                                               unsigned type_id) {
  return (static_cast<int32_t>(type_id)) << 20 | id;
}

std::pair<GAR_NAMESPACE::IdType, unsigned>
__grin_generate_id_and_type_from_int64(int64_t svr) {
  auto _type_id = static_cast<unsigned>(svr >> 20);
  auto _id = svr & 0xFFFFF;
  return {_id, _type_id};
}

GAR_NAMESPACE::IdType __grin_get_vertex_id_from_partitioned_vertex_id(
    GRIN_GRAPH_T* graph, unsigned vtype, unsigned partition_id,
    GAR_PARTITION_STRATEGY partition_strategy,
    GAR_NAMESPACE::IdType id_in_parition) {
  if (partition_strategy == SEGMENTED_PARTITION) {
    return graph->partitioned_vertex_offsets[vtype][partition_id] +
           id_in_parition;
  } else {  // HASH_PARTITION
    return graph->partition_num * id_in_parition + partition_id;
  }
}

GAR_NAMESPACE::IdType __grin_get_partitioned_vertex_id_from_vertex_id(
    GRIN_GRAPH_T* graph, unsigned vtype, unsigned partition_id,
    GAR_PARTITION_STRATEGY partition_strategy, GAR_NAMESPACE::IdType vid) {
  if (partition_strategy == SEGMENTED_PARTITION) {
    return vid - graph->partitioned_vertex_offsets[vtype][partition_id];
  } else {  // HASH_PARTITION
    return vid / graph->partition_num;
  }
}

unsigned __grin_get_master_partition_id(GRIN_GRAPH_T* graph,
                                        GAR_NAMESPACE::IdType id,
                                        unsigned type_id) {
  if (graph->partition_num == 1)  // no partition
    return 0;
  if (graph->partition_strategy == SEGMENTED_PARTITION) {
    int l = 0, r = graph->partition_num - 1;
    while (l <= r) {
      int mid = (l + r) >> 1;
      if (id >= graph->partitioned_vertex_offsets[type_id][mid] &&
          id < graph->partitioned_vertex_offsets[type_id][mid + 1]) {
        return mid;
      } else if (id < graph->partitioned_vertex_offsets[type_id][mid]) {
        r = mid - 1;
      } else {
        l = mid + 1;
      }
    }
    return l;
  } else {  // HASH_PARTITION
    return (id % graph->partition_num);
  }
}

GAR_NAMESPACE::IdType __grin_get_first_vertex_id_in_partition(
    GRIN_GRAPH_T* graph, unsigned vtype, unsigned partition_id,
    GAR_PARTITION_STRATEGY partition_strategy) {
  if (partition_strategy == SEGMENTED_PARTITION) {
    return graph->partitioned_vertex_offsets[vtype][partition_id];
  } else {  // HASH_PARTITION
    return partition_id;
  }
}

GAR_NAMESPACE::IdType __grin_get_next_vertex_id_in_partition(
    GRIN_GRAPH_T* graph, unsigned vtype, unsigned partition_id,
    GAR_PARTITION_STRATEGY partition_strategy, GAR_NAMESPACE::IdType vid) {
  if (partition_strategy == SEGMENTED_PARTITION) {
    auto p = vid + 1;
    if (p >= graph->partitioned_vertex_offsets[vtype][partition_id + 1])
      return -1;
    return p;
  } else {  // HASH_PARTITION
    auto p = vid + graph->partition_num;
    if (p >= graph->vertex_offsets[vtype + 1] - graph->vertex_offsets[vtype])
      return -1;
    return p;
  }
}
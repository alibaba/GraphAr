/*
 * Copyright 2022-2023 Alibaba Group Holding Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>

#include "yaml/Yaml.hpp"

#include "gar/graph_info.h"
#include "gar/util/filesystem.h"
#include "gar/util/yaml.h"
#include "gar/util/adj_list_type.h"
#include "gar/util/result.h"
#include "gar/util/version_parser.h"

namespace GAR_NAMESPACE_INTERNAL {

#define CHECK_HAS_ADJ_LIST_TYPE(adj_list_type) \
  do {                                         \
    if (!HasAdjacentListType(adj_list_type)) { \
      return Status::KeyError(                 \
          "Adjacency list type: ",             \
          AdjListTypeToString(adj_list_type),  \
          " is not found in edge info.");      \
    }                                          \
  } while (false)

namespace {

std::string ConcatEdgeLabel(const std::string& src_label,
                            const std::string& edge_label,
                            const std::string& dst_label) {
  return src_label + REGULAR_SEPERATOR + edge_label + REGULAR_SEPERATOR + dst_label;
}

template <int NotFoundValue = -1>
int LookupKeyIndex(const std::unordered_map<std::string, int>& key_to_index,
                    const std::string& type) {
  auto it = key_to_index.find(type);
  if (it == key_to_index.end()) {
    return NotFoundValue;
  }
  return it->second;
}

template <int NotFoundValue = -1, int DuplicateValue = -1>
int LookupNameIndex(const std::unordered_multimap<std::string, int>& name_to_index,
                    const std::string& name) {
  auto p = name_to_index.equal_range(name);
  auto it = p.first;
  if (it == p.second) {
    // Not found
    return NotFoundValue;
  }
  auto index = it->second;
  if (++it != p.second) {
    // Duplicate field name
    return DuplicateValue;
  }
  return index;
}

template <typename T>
std::vector<T> AddVectorElement(const std::vector<T>& values, 
                                T new_element) {
  std::vector<T> out;
  out.reserve(values.size() + 1);
  for (size_t i = 0; i < values.size(); ++i) {
    out.push_back(values[i]);
  }
  out.emplace_back(std::move(new_element));
  return out;
}
} // namespace

PropertyGroup::PropertyGroup(const std::vector<Property>& properties,
                             FileType file_type, const std::string& prefix)
    : properties_(properties), file_type_(file_type), prefix_(prefix) {
  if (prefix_.empty()) {
    for (const auto& p : properties_) {
      prefix_ += p.name + REGULAR_SEPERATOR;
    }
    prefix_.back() = '/';
  }
}

const std::vector<Property>& PropertyGroup::GetProperties() const {
  return properties_;
}

bool PropertyGroup::HasProperty(const std::string& property_name) const {
  for (const auto& p : properties_) {
    if (p.name == property_name) {
      return true;
    }
  }
  return false;
}

AdjacentList::AdjacentList(AdjListType type, FileType file_type,
                           const std::string& prefix)
    : type_(type), file_type_(file_type), prefix_(prefix) {
  if (prefix_.empty()) {
    prefix_ = std::string(AdjListTypeToString(type_)) + "/";
  }
}

class VertexInfo::Impl {
 public:
  Impl(const std::string& type, IdType chunk_size,
       const std::string& prefix,
       const PropertyGroupVector& property_groups,
       std::shared_ptr<const InfoVersion> version)
      : type_(type),
        chunk_size_(chunk_size),
        property_groups_(std::move(property_groups)),
        prefix_(prefix),
        version_(std::move(version)) {
    for (int i = 0; i < property_groups_.size(); i++) {
      const auto& pg = property_groups_[i];
      for (const auto& p : pg->GetProperties()) {
        property_name_to_index_.emplace(p.name, i);
        property_name_to_primary_.emplace(p.name, p.is_primary);
        property_name_to_type_.emplace(p.name, p.type);
      }
    }
  }

  bool is_validated() const noexcept {
    // TODO: check if the vertex info is valid
    return true;
  }

  std::string type_;
  IdType chunk_size_;
  PropertyGroupVector property_groups_;
  std::string prefix_;
  std::shared_ptr<const InfoVersion> version_;
  std::unordered_map<std::string, int> property_name_to_index_;
  std::unordered_map<std::string, bool> property_name_to_primary_;
  std::unordered_map<std::string, DataType> property_name_to_type_;
};

VertexInfo::VertexInfo(const std::string& label, IdType chunk_size,
                       const PropertyGroupVector& property_groups,
                       const std::string& prefix,
                       std::shared_ptr<const InfoVersion> version)
    : impl_(new Impl(label, chunk_size, prefix, property_groups, version)) {}

const std::string& VertexInfo::GetType() const { return impl_->type_; }

IdType VertexInfo::GetChunkSize() const { return impl_->chunk_size_; }

const std::string& VertexInfo::GetPrefix() const {
  return impl_->prefix_;
}

const std::shared_ptr<const InfoVersion>& VertexInfo::version() const {
  return impl_->version_;
}

Result<std::string> VertexInfo::GetFilePath(std::shared_ptr<PropertyGroup> property_group,
    IdType chunk_index) const {
  if (property_group == nullptr) {
    return Status::Invalid("property group is nullptr");
  }
  return impl_->prefix_ + property_group->GetPrefix() + "chunk" + std::to_string(chunk_index);
}

Result<std::string> VertexInfo::GetPathPrefix(std::shared_ptr<PropertyGroup> property_group) const {
  if (property_group == nullptr) {
    return Status::Invalid("property group is nullptr");
  }
  return impl_->prefix_ + property_group->GetPrefix();
}

Result<std::string> VertexInfo::GetVerticesNumFilePath() const {
  return impl_->prefix_ + "vertices_num";
}

int VertexInfo::PropertyGroupNum() const {
  return static_cast<int>(impl_->property_groups_.size());
}

std::shared_ptr<PropertyGroup> VertexInfo::GetPropertyGroup(
    const std::string& property_name) const {
  int i = LookupKeyIndex(impl_->property_name_to_index_, property_name);
  return i == -1 ? nullptr : impl_->property_groups_[i];
}

const PropertyGroupVector& VertexInfo::GetPropertyGroups() const {
  return impl_->property_groups_;
}

bool VertexInfo::IsPrimaryKey(const std::string& property_name) const {
  auto it = impl_->property_name_to_primary_.find(property_name);
  if (it == impl_->property_name_to_primary_.end()) {
    return false;
  }
  return it->second;
}

bool VertexInfo::HasProperty(const std::string& property_name) const {
  return impl_->property_name_to_index_.find(property_name) !=
         impl_->property_name_to_index_.end();
}

Result<DataType> VertexInfo::GetPropertyType(
    const std::string& property_name) const {
  auto it = impl_->property_name_to_type_.find(property_name);
  if (it == impl_->property_name_to_type_.end()) {
    return Status::Invalid("property name not found: ", property_name);
  }
  return it->second;
}

Result<std::shared_ptr<VertexInfo>> VertexInfo::AddPropertyGroup(
    std::shared_ptr<PropertyGroup> property_group) const {
  if (property_group == nullptr) {
    return Status::Invalid("property group is nullptr");
  }
  return std::make_shared<VertexInfo>(impl_->type_, impl_->chunk_size_, AddVectorElement(impl_->property_groups_, property_group), impl_->prefix_, impl_->version_);
}

bool VertexInfo::IsValidated() const {
  return impl_->is_validated();
}

Result<std::shared_ptr<VertexInfo>> VertexInfo::Load(std::shared_ptr<Yaml> yaml) {
  if (yaml == nullptr) {
    return Status::Invalid("yaml shared pointer is nullptr");
  }
  std::string vertex_type = yaml->operator[]("type").As<std::string>();
  IdType chunk_size =
      static_cast<IdType>(yaml->operator[]("chunk_size").As<int64_t>());
  std::string prefix;
  if (!yaml->operator[]("prefix").IsNone()) {
    prefix = yaml->operator[]("prefix").As<std::string>();
  }
  std::shared_ptr<const InfoVersion> version = nullptr;
  if (!yaml->operator[]("version").IsNone()) {
    GAR_ASSIGN_OR_RAISE(
        version, InfoVersion::Parse(
                     yaml->operator[]("version").As<std::string>()));
  }
  PropertyGroupVector property_groups;
  auto property_groups_node = yaml->operator[]("property_groups");
  if (!property_groups_node.IsNone()) {  // property_groups exist
    for (auto it = property_groups_node.Begin(); it != property_groups_node.End(); it++) {
      std::string pg_prefix;
      auto& node = (*it).second;
      if (!node["prefix"].IsNone()) {
        pg_prefix = node["prefix"].As<std::string>();
      }
      auto file_type = StringToFileType(node["file_type"].As<std::string>());
      std::vector<Property> property_vec;
      auto& properties = node["properties"];
      for (auto iit = properties.Begin(); iit != properties.End(); iit++) {
        Property property;
        auto& p_node = (*iit).second;
        property.name = p_node["name"].As<std::string>();
        property.type =
            DataType::TypeNameToDataType(p_node["data_type"].As<std::string>());
        property.is_primary = p_node["is_primary"].As<bool>();
        property_vec.push_back(property);
      }
      property_groups.push_back(std::make_shared<PropertyGroup>(
          property_vec, file_type, pg_prefix));
    }
  }
  return std::make_shared<VertexInfo>(vertex_type, chunk_size, property_groups, prefix, version);
}

Result<std::string> VertexInfo::Dump() const noexcept {
  if (!IsValidated()) {
    return Status::Invalid("The vertex info is not validated");
  }
  ::Yaml::Node node;
  node["type"] = impl_->type_;
  node["chunk_size"] = std::to_string(impl_->chunk_size_);
  node["prefix"] = impl_->prefix_;
  for (const auto& pg : impl_->property_groups_) {
    ::Yaml::Node pg_node;
    if (!pg->GetPrefix().empty()) {
      pg_node["prefix"] = pg->GetPrefix();
    }
    pg_node["file_type"] = FileTypeToString(pg->GetFileType());
    for (const auto& p : pg->GetProperties()) {
      ::Yaml::Node p_node;
      p_node["name"] = p.name;
      p_node["data_type"] = p.type.ToTypeName();
      p_node["is_primary"] = p.is_primary ? "true" : "false";
      pg_node["properties"].PushBack();
      pg_node["properties"][pg_node["properties"].Size() - 1] = p_node;
    }
    node["property_groups"].PushBack();
    node["property_groups"][node["property_groups"].Size() - 1] = pg_node;
  }
  if (impl_->version_ != nullptr) {
    node["version"] = impl_->version_->ToString();
  }
  std::string dump_string;
  ::Yaml::Serialize(node, dump_string);
  return dump_string;
}

Status VertexInfo::Save(const std::string& path) const {
  std::string no_url_path;
  GAR_ASSIGN_OR_RAISE(auto fs, FileSystemFromUriOrPath(path, &no_url_path));
  GAR_ASSIGN_OR_RAISE(auto yaml_content, this->Dump());
  return fs->WriteValueToFile(yaml_content, path);
}

class EdgeInfo::Impl {
 public:
  Impl(const std::string& src_type, const std::string& edge_type, const std::string& dst_type,
       IdType chunk_size, IdType src_chunk_size, IdType dst_chunk_size,
       bool directed, const std::string& prefix,
       const AdjacentListVector& adjacent_lists,
       const PropertyGroupVector& property_groups,
       std::shared_ptr<const InfoVersion> version)
    : src_type_(src_type),
      edge_type_(edge_type),
      dst_type_(dst_type),
      chunk_size_(chunk_size),
      src_chunk_size_(src_chunk_size),
      dst_chunk_size_(dst_chunk_size),
      directed_(directed),
      prefix_(prefix),
      adjacent_lists_(std::move(adjacent_lists)),
      property_groups_(std::move(property_groups)),
      version_(std::move(version)) {
    if (prefix_.empty()) {
      prefix_ = src_type_ + REGULAR_SEPERATOR + edge_type_ +
                REGULAR_SEPERATOR + dst_type_ + "/";  // default prefix
    }
    for (int i = 0; i < adjacent_lists_.size(); i++) {
      auto adj_list_type = adjacent_lists_[i]->type();
      adjacent_list_type_to_index_[adj_list_type] = i;
    }
    for (int i = 0; i < property_groups_.size(); i++) {
      const auto& pg = property_groups_[i];
      for (const auto& p : pg->GetProperties()) {
        property_name_to_index_.emplace(p.name, i);
        property_name_to_primary_.emplace(p.name, p.is_primary);
        property_name_to_type_.emplace(p.name, p.type);
      }
    }
  }

  bool is_validated() const noexcept {
    // TODO: check if the edge info is valid
    return true;
  }

  std::string src_type_;
  std::string edge_type_;
  std::string dst_type_;
  IdType chunk_size_;
  IdType src_chunk_size_;
  IdType dst_chunk_size_;
  bool directed_;
  std::string prefix_;
  AdjacentListVector adjacent_lists_; 
  PropertyGroupVector property_groups_;
  std::unordered_map<AdjListType, int> adjacent_list_type_to_index_;
  std::unordered_map<std::string, int> property_name_to_index_;
  std::unordered_map<std::string, bool> property_name_to_primary_;
  std::unordered_map<std::string, DataType> property_name_to_type_;
  std::shared_ptr<const InfoVersion> version_;
};

EdgeInfo::EdgeInfo(const std::string& src_type, const std::string& edge_type, const std::string& dst_type,
                   IdType chunk_size, IdType src_chunk_size, IdType dst_chunk_size,
                   bool directed,
                   const AdjacentListVector& adjacent_lists,
                   const PropertyGroupVector& property_groups,
                   const std::string& prefix,
                   std::shared_ptr<const InfoVersion> version)
    : impl_(new Impl(src_type, edge_type, dst_type, chunk_size, src_chunk_size, dst_chunk_size, directed, prefix, adjacent_lists, property_groups, version)) {}

const std::string& EdgeInfo::GetSrcType() const {
  return impl_->src_type_;
}

const std::string& EdgeInfo::GetEdgeType() const {
  return impl_->edge_type_;
}

const std::string& EdgeInfo::GetDstType() const {
  return impl_->dst_type_;
}

IdType EdgeInfo::GetChunkSize() const {
  return impl_->chunk_size_;
}

IdType EdgeInfo::GetSrcChunkSize() const {
  return impl_->src_chunk_size_;
}

IdType EdgeInfo::GetDstChunkSize() const {
  return impl_->dst_chunk_size_;
}

const std::string& EdgeInfo::GetPrefix() const {
  return impl_->prefix_;
}

bool EdgeInfo::IsDirected() const {
  return impl_->directed_;
}

const std::shared_ptr<const InfoVersion>& EdgeInfo::version() const {
  return impl_->version_;
} 

bool EdgeInfo::HasAdjacentListType(AdjListType adj_list_type) const {
  return impl_->adjacent_list_type_to_index_.find(adj_list_type) !=
         impl_->adjacent_list_type_to_index_.end();
}

bool EdgeInfo::HasProperty(const std::string& property_name) const {
  return impl_->property_name_to_index_.find(property_name) !=
         impl_->property_name_to_index_.end();
}

std::shared_ptr<AdjacentList> EdgeInfo::GetAdjacentList(
    AdjListType adj_list_type) const {
  auto it = impl_->adjacent_list_type_to_index_.find(adj_list_type);
  if (it == impl_->adjacent_list_type_to_index_.end()) {
    return nullptr;
  }
  return impl_->adjacent_lists_[it->second];
}

const PropertyGroupVector& EdgeInfo::GetPropertyGroups() const {
  return impl_->property_groups_;
}

std::shared_ptr<PropertyGroup> EdgeInfo::GetPropertyGroup(
    const std::string& property_name) const {
  int i = LookupKeyIndex(impl_->property_name_to_index_, property_name);
  return i == -1 ? nullptr : impl_->property_groups_[i];
}

Result<std::string> EdgeInfo::GetVerticesNumFilePath(AdjListType adj_list_type) const {
  CHECK_HAS_ADJ_LIST_TYPE(adj_list_type);
  int i = impl_->adjacent_list_type_to_index_.at(adj_list_type);
  return impl_->prefix_ + impl_->adjacent_lists_[i]->GetPrefix() + "vertex_count";
}

Result<std::string> EdgeInfo::GetEdgesNumFilePath(IdType vertex_chunk_index, AdjListType adj_list_type) const {
  CHECK_HAS_ADJ_LIST_TYPE(adj_list_type);
  int i = impl_->adjacent_list_type_to_index_.at(adj_list_type);
  return impl_->prefix_ + impl_->adjacent_lists_[i]->GetPrefix() + "edge_count" + std::to_string(vertex_chunk_index) + "_edge_count";
}

Result<std::string> EdgeInfo::GetAdjListFilePath(IdType vertex_chunk_index, IdType edge_chunk_index, AdjListType adj_list_type) const {
  CHECK_HAS_ADJ_LIST_TYPE(adj_list_type);
  int i = impl_->adjacent_list_type_to_index_.at(adj_list_type);
  return impl_->prefix_ + impl_->adjacent_lists_[i]->GetPrefix() + "adj_list/part" + std::to_string(vertex_chunk_index) + "/chunk" + std::to_string(edge_chunk_index);
}

Result<std::string> EdgeInfo::GetAdjListPathPrefix(AdjListType adj_list_type) const {
  CHECK_HAS_ADJ_LIST_TYPE(adj_list_type);
  int i = impl_->adjacent_list_type_to_index_.at(adj_list_type);
  return impl_->prefix_ + impl_->adjacent_lists_[i]->GetPrefix();
}

Result<std::string> EdgeInfo::GetAdjListOffsetFilePath(IdType vertex_chunk_index, AdjListType adj_list_type) const {
  CHECK_HAS_ADJ_LIST_TYPE(adj_list_type);
  int i = impl_->adjacent_list_type_to_index_.at(adj_list_type);
  return impl_->prefix_ + impl_->adjacent_lists_[i]->GetPrefix() + "offset/chunk" + std::to_string(vertex_chunk_index);
}

Result<std::string> EdgeInfo::GetOffsetPathPrefix(AdjListType adj_list_type) const {
  CHECK_HAS_ADJ_LIST_TYPE(adj_list_type);
  int i = impl_->adjacent_list_type_to_index_.at(adj_list_type);
  return impl_->prefix_ + impl_->adjacent_lists_[i]->GetPrefix() + "offset/";
}

Result<std::string> EdgeInfo::GetPropertyFilePath(const std::shared_ptr<PropertyGroup>& property_group,
  AdjListType adj_list_type, IdType vertex_chunk_index, IdType edge_chunk_index) const {
  if (property_group == nullptr) {
    return Status::Invalid("property group is nullptr");
  }
  int i = impl_->adjacent_list_type_to_index_.at(adj_list_type);
  return impl_->prefix_ + impl_->adjacent_lists_[i]->GetPrefix() + property_group->GetPrefix() + "part" + std::to_string(vertex_chunk_index) + "/chunk" + std::to_string(edge_chunk_index); 
}

Result<std::string> EdgeInfo::GetPropertyGroupPathPrefix(const std::shared_ptr<PropertyGroup>& property_group,
  AdjListType adj_list_type) const {
  if (property_group == nullptr) {
    return Status::Invalid("property group is nullptr");
  }
  int i = impl_->adjacent_list_type_to_index_.at(adj_list_type);
  return impl_->prefix_ + impl_->adjacent_lists_[i]->GetPrefix() + property_group->GetPrefix();
}

Result<DataType> EdgeInfo::GetPropertyType(const std::string& property_name) const {
  auto it = impl_->property_name_to_type_.find(property_name);
  if (it == impl_->property_name_to_type_.end()) {
    return Status::Invalid("property name not found: ", property_name);
  }
  return it->second;
}

bool EdgeInfo::IsPrimaryKey(const std::string& property_name) const {
  auto it = impl_->property_name_to_primary_.find(property_name);
  if (it == impl_->property_name_to_primary_.end()) {
    return false;
  }
  return it->second;
}

Result<std::shared_ptr<EdgeInfo>> EdgeInfo::AddAdjList(std::shared_ptr<AdjacentList> adj_list) const {
  if (adj_list == nullptr) {
    return Status::Invalid("adj list is nullptr");
  }
  return std::make_shared<EdgeInfo>(impl_->src_type_, impl_->edge_type_, impl_->dst_type_, impl_->chunk_size_, impl_->src_chunk_size_, impl_->dst_chunk_size_, impl_->directed_,  AddVectorElement(impl_->adjacent_lists_, adj_list), impl_->property_groups_, impl_->prefix_, impl_->version_);
}

Result<std::shared_ptr<EdgeInfo>> EdgeInfo::AddPropertyGroup(std::shared_ptr<PropertyGroup> property_group) const {
  if (property_group == nullptr) {
    return Status::Invalid("property group is nullptr");
  }
  return std::make_shared<EdgeInfo>(impl_->src_type_, impl_->edge_type_, impl_->dst_type_, impl_->chunk_size_, impl_->src_chunk_size_, impl_->dst_chunk_size_, impl_->directed_, impl_->adjacent_lists_, AddVectorElement(impl_->property_groups_, property_group), impl_->prefix_, impl_->version_);
}

bool EdgeInfo::IsValidated() const {
  return impl_->is_validated();
}

Result<std::shared_ptr<EdgeInfo>> EdgeInfo::Load(std::shared_ptr<Yaml> yaml) {
  if (yaml == nullptr) {
    return Status::Invalid("yaml shared pointer is nullptr.");
  }
  std::string src_type = yaml->operator[]("src_type").As<std::string>();
  std::string edge_type = yaml->operator[]("edge_type").As<std::string>();
  std::string dst_type = yaml->operator[]("dst_type").As<std::string>();
  IdType chunk_size =
      static_cast<IdType>(yaml->operator[]("chunk_size").As<int64_t>());
  IdType src_chunk_size =
      static_cast<IdType>(yaml->operator[]("src_chunk_size").As<int64_t>());
  IdType dst_chunk_size =
      static_cast<IdType>(yaml->operator[]("dst_chunk_size").As<int64_t>());
  bool directed = yaml->operator[]("directed").As<bool>();
  std::string prefix;
  if (!yaml->operator[]("prefix").IsNone()) {
    prefix = yaml->operator[]("prefix").As<std::string>();
  }
  std::shared_ptr<const InfoVersion> version = nullptr;
  if (!yaml->operator[]("version").IsNone()) {
    GAR_ASSIGN_OR_RAISE(
        version, InfoVersion::Parse(
                     yaml->operator[]("version").As<std::string>()));
  }

  AdjacentListVector adjacent_lists;
  PropertyGroupVector property_groups;
  auto adj_lists_node = yaml->operator[]("adj_lists");
  if (adj_lists_node.IsSequence()) {
    for (auto it = adj_lists_node.Begin(); it != adj_lists_node.End(); it++) {
      auto& node = (*it).second;
      auto ordered = node["ordered"].As<bool>();
      auto aligned = node["aligned_by"].As<std::string>();
      auto adj_list_type = OrderedAlignedToAdjListType(ordered, aligned);
      auto file_type = StringToFileType(node["file_type"].As<std::string>());
      std::string adj_list_prefix;
      if (!node["prefix"].IsNone()) {
        adj_list_prefix = node["prefix"].As<std::string>();
      }
      adjacent_lists.push_back(std::make_shared<AdjacentList>(
          adj_list_type, file_type, adj_list_prefix));
    }
  }
  auto property_groups_node = yaml->operator[]("property_groups");
  if (!property_groups_node.IsNone()) {  // property_groups exist
    for (auto pg_it = property_groups_node.Begin();
         pg_it != property_groups_node.End(); pg_it++) {
      auto& pg_node = (*pg_it).second;
      std::string pg_prefix;
      if (!pg_node["prefix"].IsNone()) {
        pg_prefix = pg_node["prefix"].As<std::string>();
      }
      auto file_type =
          StringToFileType(pg_node["file_type"].As<std::string>());
      auto properties = pg_node["properties"];
      std::vector<Property> property_vec;
      for (auto p_it = properties.Begin(); p_it != properties.End();
           p_it++) {
        auto& p_node = (*p_it).second;
        Property property;
        property.name = p_node["name"].As<std::string>();
        property.type = DataType::TypeNameToDataType(
            p_node["data_type"].As<std::string>());
        property.is_primary = p_node["is_primary"].As<bool>();
        property_vec.push_back(property);
      }
      property_groups.push_back(std::make_shared<PropertyGroup>(
          property_vec, file_type, pg_prefix));
    }
  }
  return std::make_shared<EdgeInfo>(src_type, edge_type, dst_type, chunk_size, src_chunk_size, dst_chunk_size, directed, adjacent_lists, property_groups, prefix, version);
}

Result<std::string> EdgeInfo::Dump() const noexcept {
  if (!IsValidated()) {
    return Status::Invalid("The edge info is not validated.");
  }
  ::Yaml::Node node;
  node["src_type"] = impl_->src_type_;
  node["edge_type"] = impl_->edge_type_;
  node["dst_type"] = impl_->dst_type_;
  node["chunk_size"] = std::to_string(impl_->chunk_size_);
  node["src_chunk_size"] = std::to_string(impl_->src_chunk_size_);
  node["dst_chunk_size"] = std::to_string(impl_->dst_chunk_size_);
  node["prefix"] = impl_->prefix_;
  node["directed"] = impl_->directed_ ? "true" : "false";
  for (const auto& adjacent_list : impl_->adjacent_lists_) {
    ::Yaml::Node adj_list_node;
    auto adj_list_type = adjacent_list->type();
    auto pair = AdjListTypeToOrderedAligned(adj_list_type);
    adj_list_node["ordered"] = pair.first ? "true" : "false";
    adj_list_node["aligned_by"] = pair.second;
    adj_list_node["prefix"] = adjacent_list->GetPrefix();
    adj_list_node["file_type"] =
        FileTypeToString(adjacent_list->GetFileType());
    node["adj_lists"].PushBack();
    node["adj_lists"][node["adj_lists"].Size() - 1] = adj_list_node;
  }
  for (const auto& pg : impl_->property_groups_) {
    ::Yaml::Node pg_node;
    if (!pg->GetPrefix().empty()) {
      pg_node["prefix"] = pg->GetPrefix();
    }
    pg_node["file_type"] = FileTypeToString(pg->GetFileType());
    for (const auto& p : pg->GetProperties()) {
      ::Yaml::Node p_node;
      p_node["name"] = p.name;
      p_node["data_type"] = p.type.ToTypeName();
      p_node["is_primary"] = p.is_primary ? "true" : "false";
      pg_node["properties"].PushBack();
      pg_node["properties"][pg_node["properties"].Size() - 1] = p_node;
    }
    node["property_groups"].PushBack();
    node["property_groups"][node["property_groups"].Size() - 1] = pg_node;
  }
  node["version"] = impl_->version_->ToString();
  std::string dump_string;
  ::Yaml::Serialize(node, dump_string);
  return dump_string;
}

Status EdgeInfo::Save(const std::string& path) const {
  std::string no_url_path;
  GAR_ASSIGN_OR_RAISE(auto fs, FileSystemFromUriOrPath(path, &no_url_path));
  GAR_ASSIGN_OR_RAISE(auto yaml_content, this->Dump());
  return fs->WriteValueToFile(yaml_content, path);
}

namespace {

static std::string PathToDirectory(const std::string& path) {
  if (path.rfind("s3://", 0) == 0) {
    int t = path.find_last_of('?');
    std::string prefix = path.substr(0, t);
    std::string suffix = path.substr(t);
    const size_t last_slash_idx = prefix.rfind('/');
    if (std::string::npos != last_slash_idx) {
      return prefix.substr(0, last_slash_idx + 1) + suffix;
    }
  } else {
    const size_t last_slash_idx = path.rfind('/');
    if (std::string::npos != last_slash_idx) {
      return path.substr(0, last_slash_idx + 1);  // +1 to include the slash
    }
  }
  return path;
}

static Result<std::shared_ptr<GraphInfo>> ConstructGraphInfo(
    std::shared_ptr<Yaml> graph_meta, const std::string& default_name,
    const std::string& default_prefix, const std::shared_ptr<FileSystem> fs,
    const std::string& no_url_path) {
  std::string name = default_name;
  std::string prefix = default_prefix;
  if (!graph_meta->operator[]("name").IsNone()) {
    name = graph_meta->operator[]("name").As<std::string>();
  }
  if (!graph_meta->operator[]("prefix").IsNone()) {
    prefix = graph_meta->operator[]("prefix").As<std::string>();
  }
  std::shared_ptr<const InfoVersion> version = nullptr;
  if (!graph_meta->operator[]("version").IsNone()) {
    GAR_ASSIGN_OR_RAISE(
        version, InfoVersion::Parse(
                     graph_meta->operator[]("version").As<std::string>()));
  }

  VertexInfoVector vertex_infos;
  EdgeInfoVector edge_infos;
  const auto& vertices = graph_meta->operator[]("vertices");
  if (vertices.IsSequence()) {
    for (auto it = vertices.Begin(); it != vertices.End(); it++) {
      std::string vertex_meta_file =
          no_url_path + (*it).second.As<std::string>();
      GAR_ASSIGN_OR_RAISE(auto input,
                          fs->ReadFileToValue<std::string>(vertex_meta_file));
      GAR_ASSIGN_OR_RAISE(auto vertex_meta, Yaml::Load(input));
      GAR_ASSIGN_OR_RAISE(auto vertex_info, VertexInfo::Load(vertex_meta));
      vertex_infos.push_back(vertex_info);
    }
  }
  const auto& edges = graph_meta->operator[]("edges");
  if (edges.IsSequence()) {
    for (auto it = edges.Begin(); it != edges.End(); it++) {
      std::string edge_meta_file = no_url_path + (*it).second.As<std::string>();
      GAR_ASSIGN_OR_RAISE(auto input,
                          fs->ReadFileToValue<std::string>(edge_meta_file));
      GAR_ASSIGN_OR_RAISE(auto edge_meta, Yaml::Load(input));
      GAR_ASSIGN_OR_RAISE(auto edge_info, EdgeInfo::Load(edge_meta));
      edge_infos.push_back(edge_info);
    }
  }
  return std::make_shared<GraphInfo>(name, vertex_infos, edge_infos, prefix, version);
}

}  // namespace

class GraphInfo::Impl {
 public:
  Impl(const std::string& graph_name, VertexInfoVector vertex_infos,
       EdgeInfoVector edge_infos, const std::string& prefix,
       std::shared_ptr<const InfoVersion> version)
      : name_(graph_name),
        vertex_infos_(std::move(vertex_infos)),
        edge_infos_(std::move(edge_infos)),
        prefix_(prefix),
        version_(std::move(version)) {
    for (int i = 0; i < vertex_infos_.size(); i++) {
      vlabel_to_index_[vertex_infos_[i]->GetType()] = i;
    }
    for (int i = 0; i < edge_infos_.size(); i++) {
      std::string edge_label = ConcatEdgeLabel(
          edge_infos_[i]->GetSrcType(), edge_infos_[i]->GetEdgeType(),
          edge_infos_[i]->GetDstType());
      elabel_to_index_[edge_label] = i;
    }
  }
  
  bool is_validated() const noexcept {
    return !name_.empty() && !prefix_.empty() && version_ != nullptr &&
           !vertex_infos_.empty() && !edge_infos_.empty();
  }

  std::string name_;
  VertexInfoVector vertex_infos_;
  EdgeInfoVector edge_infos_;
  std::string prefix_;
  std::shared_ptr<const InfoVersion> version_;
  std::unordered_map<std::string, int> vlabel_to_index_;
  std::unordered_map<std::string, int> elabel_to_index_;
};

GraphInfo::GraphInfo(const std::string& graph_name,
                     VertexInfoVector vertex_infos,
                     EdgeInfoVector edge_infos,
                     const std::string& prefix,
                     std::shared_ptr<const InfoVersion> version)
    : impl_(new Impl(graph_name, std::move(vertex_infos),
                     std::move(edge_infos), prefix, version)) {}

const std::string& GraphInfo::GetName() const { return impl_->name_; }

const std::string& GraphInfo::GetPrefix() const {
  return impl_->prefix_;
}

const std::shared_ptr<const InfoVersion>& GraphInfo::version() const {
  return impl_->version_;
}

std::shared_ptr<VertexInfo> GraphInfo::GetVertexInfoByType(
    const std::string& type) const {
  int i = GetVertexInfoIndex(type);
  return i == -1 ? nullptr : impl_->vertex_infos_[i];
}

int GraphInfo::GetVertexInfoIndex(const std::string& type) const {
  return LookupKeyIndex(impl_->vlabel_to_index_, type);
}

std::shared_ptr<EdgeInfo> GraphInfo::GetEdgeInfoByType(
    const std::string& src_type, const std::string& edge_type, const std::string& dst_type) const {
  int i = GetEdgeInfoIndex(src_type, edge_type, dst_type);
  return i == -1 ? nullptr : impl_->edge_infos_[i];
}

int GraphInfo::GetEdgeInfoIndex(const std::string& src_type, const std::string& edge_type, const std::string& dst_type) const {
  std::string edge_label = ConcatEdgeLabel(src_type, edge_type, dst_type);
  return LookupKeyIndex(impl_->elabel_to_index_, edge_label);
}

int GraphInfo::VertexInfoNum() const {
  return static_cast<int>(impl_->vertex_infos_.size());
}

int GraphInfo::EdgeInfoNum() const {
  return static_cast<int>(impl_->edge_infos_.size());
}

const std::shared_ptr<VertexInfo> GraphInfo::GetVertexInfoByIndex(int index) const {
  if (index < 0 || index >= impl_->vertex_infos_.size()) {
    return nullptr;
  }
  return impl_->vertex_infos_[index];
}

const std::shared_ptr<EdgeInfo> GraphInfo::GetEdgeInfoByIndex(int index) const {
  if (index < 0 || index >= impl_->edge_infos_.size()) {
    return nullptr;
  }
  return impl_->edge_infos_[index];
}

const VertexInfoVector& GraphInfo::GetVertexInfos() const {
  return impl_->vertex_infos_;
}

const EdgeInfoVector& GraphInfo::GetEdgeInfos() const {
  return impl_->edge_infos_;
}

bool GraphInfo::IsValidated() const {
  return impl_->is_validated();
}

Result<std::shared_ptr<GraphInfo>> GraphInfo::AddVertex(std::shared_ptr<VertexInfo> vertex_info) const {
  if (vertex_info == nullptr) {
    return Status::Invalid("vertex info is nullptr");
  }
  if (GetVertexInfoIndex(vertex_info->GetType()) != -1) {
    return Status::Invalid("vertex info already exists");
  }
  return std::make_shared<GraphInfo>(impl_->name_, AddVectorElement(impl_->vertex_infos_, vertex_info), impl_->edge_infos_, impl_->prefix_, impl_->version_);
}

Result<std::shared_ptr<GraphInfo>> GraphInfo::AddEdge(std::shared_ptr<EdgeInfo> edge_info) const {
  if (edge_info == nullptr) {
    return Status::Invalid("edge info is nullptr");
  }
  if (GetEdgeInfoIndex(edge_info->GetSrcType(), edge_info->GetEdgeType(), edge_info->GetDstType()) != -1) {
    return Status::Invalid("edge info already exists");
  }
  return std::make_shared<GraphInfo>(impl_->name_, impl_->vertex_infos_, AddVectorElement(impl_->edge_infos_, edge_info), impl_->prefix_, impl_->version_);
}

Result<std::shared_ptr<GraphInfo>> GraphInfo::Load(const std::string& path) {
  std::string no_url_path;
  GAR_ASSIGN_OR_RAISE(auto fs, FileSystemFromUriOrPath(path, &no_url_path));
  GAR_ASSIGN_OR_RAISE(auto yaml_content,
                      fs->ReadFileToValue<std::string>(no_url_path));
  GAR_ASSIGN_OR_RAISE(auto graph_meta, Yaml::Load(yaml_content));
  std::string default_name = "graph";
  std::string default_prefix = PathToDirectory(path);
  no_url_path = PathToDirectory(no_url_path);
  return ConstructGraphInfo(graph_meta, default_name, default_prefix, fs,
                            no_url_path);
}

Result<std::shared_ptr<GraphInfo>> GraphInfo::Load(const std::string& input,
                                  const std::string& relative_location) {
  GAR_ASSIGN_OR_RAISE(auto graph_meta, Yaml::Load(input));
  std::string default_name = "graph";
  std::string default_prefix =
      relative_location;  // default chunk file prefix is relative location
  std::string no_url_path;
  GAR_ASSIGN_OR_RAISE(auto fs,
                      FileSystemFromUriOrPath(relative_location, &no_url_path));
  return ConstructGraphInfo(graph_meta, default_name, default_prefix, fs,
                            no_url_path);
}

Result<std::string> GraphInfo::Dump() const {
  if (!IsValidated()) {
    return Status::Invalid("The graph info is not validated.");
  }
  ::Yaml::Node node;
  node["name"] = impl_->name_;
  node["prefix"] = impl_->prefix_;
  node["vertices"];
  node["edges"];
  for (const auto& vertex : GetVertexInfos()) {
    node["vertices"].PushBack();
    node["vertices"][node["vertices"].Size() - 1] = vertex->GetType() + ".vertex.yaml";
  }
  for (const auto& edge : GetEdgeInfos()) {
    node["edges"].PushBack();
    node["edges"][node["edges"].Size() - 1] = ConcatEdgeLabel(edge->GetSrcType(), edge->GetEdgeType(), edge->GetDstType()) + ".edge.yaml";
  }
  if (impl_->version_ != nullptr) {
    node["version"] = impl_->version_->ToString();
  }
  std::string dump_string;
  ::Yaml::Serialize(node, dump_string);
  return dump_string;
}

Status GraphInfo::Save(const std::string& path) const {
  std::string no_url_path;
  GAR_ASSIGN_OR_RAISE(auto fs, FileSystemFromUriOrPath(path, &no_url_path));
  GAR_ASSIGN_OR_RAISE(auto yaml_content, this->Dump());
  return fs->WriteValueToFile(yaml_content, no_url_path);
}

}  // namespace GAR_NAMESPACE_INTERNAL

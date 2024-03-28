/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

package org.apache.graphar.info.yaml;

import java.util.ArrayList;
import java.util.List;

public class EdgeYamlParser {
    private String src_label;
    private String edge_label;
    private String dst_label;
    private long chunk_size;
    private long src_chunk_size;
    private long dst_chunk_size;
    private boolean directed;
    private String prefix;
    private List<AdjacentListYamlParser> adjacent_lists;
    private List<PropertyGroupYamlParser> property_groups;
    private String version;

    public EdgeYamlParser() {
        this.src_label = "";
        this.edge_label = "";
        this.dst_label = "";
        this.chunk_size = 0;
        this.src_chunk_size = 0;
        this.dst_chunk_size = 0;
        this.directed = false;
        this.prefix = "";
        this.adjacent_lists = new ArrayList<>();
        this.property_groups = new ArrayList<>();
        this.version = "";
    }

    public String getSrc_label() {
        return src_label;
    }

    public void setSrc_label(String src_label) {
        this.src_label = src_label;
    }

    public String getEdge_label() {
        return edge_label;
    }

    public void setEdge_label(String edge_label) {
        this.edge_label = edge_label;
    }

    public String getDst_label() {
        return dst_label;
    }

    public void setDst_label(String dst_label) {
        this.dst_label = dst_label;
    }

    public boolean isDirected() {
        return directed;
    }

    public void setDirected(boolean directed) {
        this.directed = directed;
    }

    public long getChunk_size() {
        return chunk_size;
    }

    public void setChunk_size(long chunk_size) {
        this.chunk_size = chunk_size;
    }

    public long getSrc_chunk_size() {
        return src_chunk_size;
    }

    public void setSrc_chunk_size(long src_chunk_size) {
        this.src_chunk_size = src_chunk_size;
    }

    public long getDst_chunk_size() {
        return dst_chunk_size;
    }

    public void setDst_chunk_size(long dst_chunk_size) {
        this.dst_chunk_size = dst_chunk_size;
    }

    public String getPrefix() {
        return prefix;
    }

    public void setPrefix(String prefix) {
        this.prefix = prefix;
    }

    public List<AdjacentListYamlParser> getAdjacent_lists() {
        return adjacent_lists;
    }

    public void setAdjacent_lists(List<AdjacentListYamlParser> adjacent_lists) {
        this.adjacent_lists = adjacent_lists;
    }

    public List<PropertyGroupYamlParser> getProperty_groups() {
        return property_groups;
    }

    public void setProperty_groups(List<PropertyGroupYamlParser> property_groups) {
        this.property_groups = property_groups;
    }

    public String getVersion() {
        return version;
    }

    public void setVersion(String version) {
        this.version = version;
    }
}

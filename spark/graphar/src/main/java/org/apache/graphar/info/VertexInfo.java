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

package org.apache.graphar.info;

import org.apache.graphar.info.type.DataType;
import org.apache.graphar.info.yaml.VertexYamlParser;
import java.io.IOException;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.yaml.snakeyaml.LoaderOptions;
import org.yaml.snakeyaml.Yaml;
import org.yaml.snakeyaml.constructor.Constructor;

public class VertexInfo {
    private final String label;
    private final long chunkSize;
    private final PropertyGroups propertyGroups;
    private final String prefix;
    private final String version;

    public VertexInfo(
            String label,
            long chunkSize,
            List<PropertyGroup> propertyGroups,
            String prefix,
            String version) {
        this.label = label;
        this.chunkSize = chunkSize;
        this.propertyGroups = new PropertyGroups(propertyGroups);
        this.prefix = prefix;
        this.version = version;
    }

    private VertexInfo(VertexYamlParser parser) {
        this(
                parser.getLabel(),
                parser.getChunk_size(),
                parser.getProperty_groups().stream()
                        .map(PropertyGroup::new)
                        .collect(Collectors.toUnmodifiableList()),
                parser.getPrefix(),
                parser.getVersion());
    }

    private VertexInfo(
            String label,
            long chunkSize,
            PropertyGroups propertyGroups,
            String prefix,
            String version) {
        this.label = label;
        this.chunkSize = chunkSize;
        this.propertyGroups = propertyGroups;
        this.prefix = prefix;
        this.version = version;
    }

    public static VertexInfo load(String vertexInfoPath, Configuration conf) throws IOException {
        if (conf == null) {
            conf = new Configuration();
        }
        Path path = new Path(vertexInfoPath);
        FileSystem fileSystem = path.getFileSystem(conf);
        FSDataInputStream inputStream = fileSystem.open(path);
        Yaml vertexInfoYamlLoader =
                new Yaml(new Constructor(VertexYamlParser.class, new LoaderOptions()));
        VertexYamlParser vertexInfoYaml = vertexInfoYamlLoader.load(inputStream);
        return new VertexInfo(vertexInfoYaml);
    }

    Optional<VertexInfo> addPropertyGroupAsNew(PropertyGroup propertyGroup) {
        return propertyGroups
                .addPropertyGroupAsNew(propertyGroup)
                .map(
                        newPropertyGroups ->
                                new VertexInfo(
                                        label, chunkSize, newPropertyGroups, prefix, version));
    }

    int propertyGroupNum() {
        return propertyGroups.getPropertyGroupNum();
    }

    PropertyGroup getPropertyGroup(String propertyName) {
        return propertyGroups.getPropertyGroup(propertyName);
    }

    DataType getPropertyType(String propertyName) {
        return propertyGroups.getPropertyType(propertyName);
    }

    boolean hasProperty(String propertyName) {
        return propertyGroups.hasProperty(propertyName);
    }

    boolean isPrimaryKey(String propertyName) {
        return propertyGroups.isPrimaryKey(propertyName);
    }

    boolean isNullableKey(String propertyName) {
        return propertyGroups.isNullableKey(propertyName);
    }

    boolean hasPropertyGroup(PropertyGroup propertyGroup) {
        return propertyGroups.hasPropertyGroup(propertyGroup);
    }

    // TODO(@Thespica): Implement file path get methods
    //
    //    String getFilePath(PropertyGroup propertyGroup,
    //                                    long chunkIndex) {
    //
    //    }
    //
    //    String getPathPrefix(
    //            PropertyGroup propertyGroup) {
    //
    //    }
    //
    //    String getVerticesNumFilePath() {
    //
    //    }

    // TODO(@Thespica): Implement save and dump methods
    //
    //    void save(String fileName) {
    //
    //    }
    //
    //    String Dump() {
    //
    //    }

    public String getLabel() {
        return label;
    }

    public long getChunkSize() {
        return chunkSize;
    }

    public List<PropertyGroup> getPropertyGroups() {
        return propertyGroups.getPropertyGroupList();
    }

    public String getPrefix() {
        return prefix;
    }

    public String getVersion() {
        return version;
    }
}

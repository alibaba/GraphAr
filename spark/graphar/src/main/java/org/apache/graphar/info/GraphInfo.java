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

import org.apache.graphar.info.yaml.GraphYamlParser;
import org.apache.graphar.util.GeneralParams;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.function.Function;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.yaml.snakeyaml.LoaderOptions;
import org.yaml.snakeyaml.Yaml;
import org.yaml.snakeyaml.constructor.Constructor;

public class GraphInfo {
    private final String name;
    private final List<VertexInfo> vertexInfos;
    private final List<EdgeInfo> edgeInfos;
    private final String prefix;
    private final Map<String, VertexInfo> vertexLabel2VertexInfo;
    private final Map<String, EdgeInfo> edgeConcat2EdgeInfo;
    private final String version;

    public GraphInfo(
            String name,
            List<VertexInfo> vertexInfos,
            List<EdgeInfo> edgeInfos,
            String prefix,
            String version) {
        this.name = name;
        this.vertexInfos = List.copyOf(vertexInfos);
        this.edgeInfos = List.copyOf(edgeInfos);
        this.prefix = prefix;
        this.version = version;
        this.vertexLabel2VertexInfo =
                vertexInfos.stream()
                        .collect(
                                Collectors.toUnmodifiableMap(
                                        VertexInfo::getLabel, Function.identity()));
        this.edgeConcat2EdgeInfo =
                edgeInfos.stream()
                        .collect(
                                Collectors.toUnmodifiableMap(
                                        EdgeInfo::getConcat, Function.identity()));
    }

    private GraphInfo(GraphYamlParser graphYaml, Configuration conf) throws IOException {
        this(
                graphYaml.getName(),
                vertexFileNames2VertexInfos(graphYaml.getVertices(), conf),
                edgeFileNames2EdgeInfos(graphYaml.getEdges(), conf),
                graphYaml.getPrefix(),
                graphYaml.getVersion());
    }

    private GraphInfo(
            String name,
            List<VertexInfo> vertexInfos,
            List<EdgeInfo> edgeInfos,
            String prefix,
            Map<String, VertexInfo> vertexLabel2VertexInfo,
            Map<String, EdgeInfo> edgeConcat2EdgeInfo,
            String version) {
        this.name = name;
        this.vertexInfos = vertexInfos;
        this.edgeInfos = edgeInfos;
        this.prefix = prefix;
        this.vertexLabel2VertexInfo = vertexLabel2VertexInfo;
        this.edgeConcat2EdgeInfo = edgeConcat2EdgeInfo;
        this.version = version;
    }

    public static GraphInfo load(String graphPath) throws IOException {
        return load(graphPath, null);
    }

    public static GraphInfo load(String graphPath, Configuration conf) throws IOException {
        if (graphPath == null) {
            conf = new Configuration();
        }
        Path path = new Path(graphPath);
        FileSystem fileSystem = path.getFileSystem(conf);
        FSDataInputStream inputStream = fileSystem.open(path);
        Yaml graphYamlLoader =
                new Yaml(new Constructor(GraphYamlParser.class, new LoaderOptions()));
        GraphYamlParser graphYaml = graphYamlLoader.load(inputStream);
        return new GraphInfo(graphYaml, conf);
    }

    // TODO(@Thespica): Implement save and dump methods
    //    public void save(String path) {
    //
    //    }
    //
    //    public String dump() {
    //
    //    }

    Optional<GraphInfo> addVertexAsNew(VertexInfo vertexInfo) {
        if (vertexInfo == null || hasVertexInfo(vertexInfo.getLabel())) {
            return Optional.empty();
        }
        List<VertexInfo> newVertexInfos =
                Stream.concat(vertexInfos.stream(), Stream.of(vertexInfo))
                        .collect(Collectors.toList());
        Map<String, VertexInfo> newVertexLabel2VertexInfo =
                Stream.concat(
                                vertexLabel2VertexInfo.entrySet().stream(),
                                Stream.of(Map.entry(vertexInfo.getLabel(), vertexInfo)))
                        .collect(
                                Collectors.toUnmodifiableMap(
                                        Map.Entry::getKey, Map.Entry::getValue));
        return Optional.of(
                new GraphInfo(
                        name,
                        newVertexInfos,
                        edgeInfos,
                        prefix,
                        newVertexLabel2VertexInfo,
                        edgeConcat2EdgeInfo,
                        version));
    }

    Optional<GraphInfo> addEdgeAsNew(EdgeInfo edgeInfo) {
        if (edgeInfo == null
                || hasEdgeInfo(
                        edgeInfo.getSrcLabel(), edgeInfo.getEdgeLabel(), edgeInfo.getDstLabel())) {
            return Optional.empty();
        }
        List<EdgeInfo> newEdgeInfos =
                Stream.concat(edgeInfos.stream(), Stream.of(edgeInfo)).collect(Collectors.toList());
        Map<String, EdgeInfo> newEdgeConcat2EdgeInfo =
                Stream.concat(
                                edgeConcat2EdgeInfo.entrySet().stream(),
                                Stream.of(Map.entry(edgeInfo.getConcat(), edgeInfo)))
                        .collect(
                                Collectors.toUnmodifiableMap(
                                        Map.Entry::getKey, Map.Entry::getValue));
        return Optional.of(
                new GraphInfo(
                        name,
                        vertexInfos,
                        newEdgeInfos,
                        prefix,
                        vertexLabel2VertexInfo,
                        newEdgeConcat2EdgeInfo,
                        version));
    }

    public boolean hasVertexInfo(String label) {
        return vertexLabel2VertexInfo.containsKey(label);
    }

    public boolean hasEdgeInfo(String srcLabel, String edgeLabel, String dstLabel) {
        return edgeConcat2EdgeInfo.containsKey(srcLabel + dstLabel + edgeLabel);
    }

    public VertexInfo getVertexInfo(String label) {
        checkVertexExist(label);
        return vertexLabel2VertexInfo.get(label);
    }

    public EdgeInfo getEdgeInfo(String srcLabel, String edgeLabel, String dstLabel) {
        checkEdgeExist(srcLabel, edgeLabel, dstLabel);
        return edgeConcat2EdgeInfo.get(srcLabel + edgeLabel + dstLabel);
    }

    public int getVertexInfoNum() {
        return vertexInfos.size();
    }

    public int getEdgeInfoNum() {
        return edgeInfos.size();
    }

    public String getName() {
        return name;
    }

    public List<VertexInfo> getVertexInfos() {
        return vertexInfos;
    }

    public List<EdgeInfo> getEdgeInfos() {
        return edgeInfos;
    }

    public String getPrefix() {
        return prefix;
    }

    public String getVersion() {
        return version;
    }

    private static List<VertexInfo> vertexFileNames2VertexInfos(
            List<String> vertexFileNames, Configuration conf) throws IOException {
        ArrayList<VertexInfo> tempVertices = new ArrayList<>(vertexFileNames.size());
        for (String vertexFileName : vertexFileNames) {
            tempVertices.add(VertexInfo.load(vertexFileName, conf));
        }
        return List.copyOf(tempVertices);
    }

    private static List<EdgeInfo> edgeFileNames2EdgeInfos(
            List<String> edgeFileNames, Configuration conf) throws IOException {
        ArrayList<EdgeInfo> tempEdges = new ArrayList<>(edgeFileNames.size());
        for (String edgeFileName : edgeFileNames) {
            tempEdges.add(EdgeInfo.load(edgeFileName, conf));
        }
        return List.copyOf(tempEdges);
    }

    private void checkVertexExist(String label) {
        if (!hasVertexInfo(label)) {
            throw new IllegalArgumentException(
                    "Vertex label " + label + " not exist in graph " + getName());
        }
    }

    private void checkEdgeExist(String srcLabel, String dstLabel, String edgeLabel) {
        if (!hasEdgeInfo(srcLabel, dstLabel, edgeLabel)) {
            throw new IllegalArgumentException(
                    "Edge label"
                            + srcLabel
                            + GeneralParams.regularSeparator
                            + GeneralParams.regularSeparator
                            + edgeLabel
                            + GeneralParams.regularSeparator
                            + dstLabel
                            + " not exist in graph "
                            + getName());
        }
    }
}

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
import org.apache.graphar.info.type.FileType;
import org.apache.graphar.info.yaml.PropertyGroupYamlParser;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.function.Function;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import org.apache.graphar.util.GeneralParams;
import org.jetbrains.annotations.NotNull;

public class PropertyGroup implements Iterable<Property> {
    private final List<Property> propertyList;
    private final Map<String, Property> propertyMap;
    private final FileType fileType;
    private final String prefix;

    public PropertyGroup(List<Property> propertyMap, FileType fileType, String prefix) {
        this.propertyList = List.copyOf(propertyMap);
        this.propertyMap =
                propertyMap.stream()
                        .collect(
                                Collectors.toUnmodifiableMap(
                                        Property::getName, Function.identity()));
        this.fileType = fileType;
        this.prefix = prefix;
    }

    PropertyGroup(PropertyGroupYamlParser yamlParser) {
        this(
                yamlParser.getProperties().stream()
                        .map(Property::new)
                        .collect(Collectors.toUnmodifiableList()),
                FileType.fromString(yamlParser.getFile_type()),
                yamlParser.getPrefix());
    }

    public Optional<PropertyGroup> addPropertyAsNew(Property property) {
        if (property == null || propertyMap.containsKey(property.getName())) {
            return Optional.empty();
        }
        List<Property> newPropertyMap =
                Stream.concat(propertyMap.values().stream(), Stream.of(property))
                        .collect(Collectors.toUnmodifiableList());
        return Optional.of(new PropertyGroup(newPropertyMap, fileType, prefix));
    }

    @NotNull
    @Override
    public Iterator<Property> iterator() {
        return propertyList.iterator();
    }

    @Override
    public String toString() {
        return propertyList.stream().map(Property::getName).collect(Collectors.joining(GeneralParams.regularSeparator));
    }

    public int size() {
        return propertyList.size();
    }

    public List<Property> getPropertyList() {
        return propertyList;
    }

    public Map<String, Property> getPropertyMap() {
        return propertyMap;
    }

    public FileType getFileType() {
        return fileType;
    }

    public String getPrefix() {
        return prefix;
    }
}

class PropertyGroups {
    private final List<PropertyGroup> propertyGroupList;
    private final Map<String, PropertyGroup> propertyGroupMap;
    private final Map<String, Property> properties;

    PropertyGroups(List<PropertyGroup> propertyGroupList) {
        this.propertyGroupList = List.copyOf(propertyGroupList);
        this.properties =
                propertyGroupList.stream()
                        .flatMap(propertyGroup -> propertyGroup.getPropertyMap().values().stream())
                        .collect(
                                Collectors.toUnmodifiableMap(
                                        Property::getName, Function.identity()));
        HashMap<String, PropertyGroup> tempPropertyGroupMap = new HashMap<>(this.properties.size());
        for (PropertyGroup propertyGroup : propertyGroupList) {
            for (Property property : propertyGroup) {
                tempPropertyGroupMap.put(property.getName(), propertyGroup);
            }
        }
        this.propertyGroupMap = Map.copyOf(tempPropertyGroupMap);
    }

    private PropertyGroups(
            List<PropertyGroup> propertyGroupList,
            Map<String, PropertyGroup> propertyGroupMap,
            Map<String, Property> properties) {
        this.propertyGroupList = propertyGroupList;
        this.propertyGroupMap = propertyGroupMap;
        this.properties = properties;
    }

    Optional<PropertyGroups> addPropertyGroupAsNew(PropertyGroup propertyGroup) {
        if (propertyGroup == null || propertyGroup.size() == 0 || hasPropertyGroup(propertyGroup)) {
            return Optional.empty();
        }
        for (Property property : propertyGroup) {
            if (hasProperty(property.getName())) {
                return Optional.empty();
            }
        }
        List<PropertyGroup> newPropertyGroupsAsList =
                Stream.concat(propertyGroupList.stream(), Stream.of(propertyGroup))
                        .collect(Collectors.toUnmodifiableList());
        Map<String, PropertyGroup> tempPropertyGroupAsMap = new HashMap<>(propertyGroupMap);
        for (Property property : propertyGroup) {
            tempPropertyGroupAsMap.put(property.getName(), propertyGroup);
        }
        Map<String, Property> newProperties =
                Stream.concat(
                                properties.values().stream(),
                                propertyGroup.getPropertyMap().values().stream())
                        .collect(
                                Collectors.toUnmodifiableMap(
                                        Property::getName, Function.identity()));
        return Optional.of(
                new PropertyGroups(
                        newPropertyGroupsAsList,
                        Map.copyOf(tempPropertyGroupAsMap),
                        newProperties));
    }

    boolean hasProperty(String propertyName) {
        return properties.containsKey(propertyName);
    }

    boolean hasPropertyGroup(PropertyGroup propertyGroup) {
        return propertyGroupList.contains(propertyGroup);
    }

    int getPropertyGroupNum() {
        return propertyGroupList.size();
    }

    DataType getPropertyType(String propertyName) {
        checkPropertyExist(propertyName);
        return properties.get(propertyName).getDataType();
    }

    boolean isPrimaryKey(String propertyName) {
        checkPropertyExist(propertyName);
        return properties.get(propertyName).isPrimary();
    }

    boolean isNullableKey(String propertyName) {
        checkPropertyExist(propertyName);
        return properties.get(propertyName).isNullable();
    }

    List<PropertyGroup> getPropertyGroupList() {
        return propertyGroupList;
    }

    Map<String, PropertyGroup> getPropertyGroupMap() {
        return propertyGroupMap;
    }

    PropertyGroup getPropertyGroup(String propertyName) {
        checkPropertyExist(propertyName);
        return propertyGroupMap.get(propertyName);
    }

    Map<String, Property> getProperties() {
        return properties;
    }

    Property getProperty(String propertyName) {
        checkPropertyExist(propertyName);
        return properties.get(propertyName);
    }

    private void checkPropertyExist(String propertyName) {
        if (!hasProperty(propertyName)) {
            throw new IllegalArgumentException(
                    "Property " + propertyName + " does not exist in the property group " + this);
        }
    }
}

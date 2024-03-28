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
import org.apache.graphar.info.yaml.PropertyYamlParser;

public class Property {
    private final String name;
    private final DataType dataType;
    private final boolean primary;
    private final boolean nullable;

    public Property(String name, DataType dataType, boolean primary, boolean nullable) {
        this.name = name;
        this.dataType = dataType;
        this.primary = primary;
        this.nullable = nullable;
    }

    Property(PropertyYamlParser yamlParser) {
        this.name = yamlParser.getName();
        this.dataType = DataType.fromString(yamlParser.getData_type());
        this.primary = yamlParser.getIs_primary();
        this.nullable = yamlParser.getIs_nullable();
    }

    public String getName() {
        return name;
    }

    public DataType getDataType() {
        return dataType;
    }

    public boolean isPrimary() {
        return primary;
    }

    public boolean isNullable() {
        return nullable;
    }
}

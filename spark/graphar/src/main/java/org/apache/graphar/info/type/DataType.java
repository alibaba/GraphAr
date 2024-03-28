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

package org.apache.graphar.info.type;

public enum DataType {
    /** Boolean */
    BOOL,

    /** Signed 32-bit integer */
    INT32,

    /** Signed 64-bit integer */
    INT64,

    /** 4-byte floating point value */
    FLOAT,

    /** 8-byte floating point value */
    DOUBLE,

    /** UTF8 variable-length string */
    STRING,

    /** List of same type */
    LIST;

    public static DataType fromString(String s) {
        switch (s) {
            case "bool":
                return BOOL;
            case "int32":
                return INT32;
            case "int64":
                return INT64;
            case "float":
                return FLOAT;
            case "double":
                return DOUBLE;
            case "string":
                return STRING;
            case "list":
                return LIST;
            default:
                throw new IllegalArgumentException("Unknown data type: " + s);
        }
    }

    @Override
    public String toString() {
        return name().toLowerCase();
    }
}

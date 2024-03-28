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

public enum AdjListType {
    /** collection of edges by source, but unordered, can represent COO format */
    unordered_by_source,
    /** collection of edges by destination, but unordered, can represent COO */
    unordered_by_dest,
    /** collection of edges by source, ordered by source, can represent CSR format */
    ordered_by_source,
    /** collection of edges by destination, ordered by destination, can represent CSC format */
    ordered_by_dest;

    public static AdjListType fromOrderedAndAlignedBy(boolean ordered, String alignedBy) {
        switch (alignedBy) {
            case "src":
                return ordered ? ordered_by_source : unordered_by_source;
            case "dst":
                return ordered ? ordered_by_dest : unordered_by_dest;
            default:
                throw new IllegalArgumentException("Invalid alignedBy: " + alignedBy);
        }
    }
}

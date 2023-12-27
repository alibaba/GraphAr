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

package com.alibaba.graphar.util;

import static com.alibaba.graphar.util.CppClassName.GAR_UTIL_INDEX_CONVERTER;
import static com.alibaba.graphar.util.CppHeaderName.GAR_UTIL_UTIL_H;

import com.alibaba.fastffi.CXXHead;
import com.alibaba.fastffi.CXXPointer;
import com.alibaba.fastffi.FFIFactory;
import com.alibaba.fastffi.FFIGen;
import com.alibaba.fastffi.FFITypeAlias;

/** The iterator for traversing a type of edges. */
@FFIGen
@FFITypeAlias(GAR_UTIL_INDEX_CONVERTER)
@CXXHead(GAR_UTIL_UTIL_H)
public interface IndexConverter extends CXXPointer {
    @FFIFactory
    interface Factory {
        // IndexConverter create(@FFITypeAlias("std::vector<GrapharChive::IdType>&&")
        // StdVector<Long> edgeChunkNums);
    }
}

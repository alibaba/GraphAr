// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

// Package types holds the primitive value types shared across the GraphAr Go
// SDK: FileType, AdjListType, Cardinality, and InfoVersion.
//
// All types in this package are value-semantic (no hidden state, safe to copy)
// and have no external dependencies, so any other package in the SDK may
// import them without risk of cycles.
//
// Parsing helpers (e.g. ParseFileType) accept the on-disk spellings used by the
// GraphAr YAML schema, so values round-trip across implementations.
package types

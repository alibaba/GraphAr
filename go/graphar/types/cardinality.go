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

package types

import "fmt"

// Cardinality is the multiplicity of a property's value. SINGLE is the default
// for newly constructed properties; LIST and SET are only legal on vertex
// properties (edge properties must be single-valued).
type Cardinality uint8

// Cardinality values.
const (
	// CardinalitySingle is one value per record.
	CardinalitySingle Cardinality = iota
	// CardinalityList is an ordered, possibly repeating list of values per record.
	CardinalityList
	// CardinalitySet is an unordered set of distinct values per record.
	CardinalitySet
)

// On-disk spellings, shared by String (write) and ParseCardinality (read) so
// the two cannot drift.
const (
	cardinalitySingleName = "single"
	cardinalityListName   = "list"
	cardinalitySetName    = "set"
)

// String returns the on-disk spelling.
func (c Cardinality) String() string {
	switch c {
	case CardinalitySingle:
		return cardinalitySingleName
	case CardinalityList:
		return cardinalityListName
	case CardinalitySet:
		return cardinalitySetName
	default:
		return "unknown"
	}
}

// ParseCardinality parses the on-disk spelling produced by Cardinality.String.
func ParseCardinality(s string) (Cardinality, error) {
	switch s {
	case cardinalitySingleName:
		return CardinalitySingle, nil
	case cardinalityListName:
		return CardinalityList, nil
	case cardinalitySetName:
		return CardinalitySet, nil
	default:
		return 0, fmt.Errorf("%w: %q", ErrInvalidCardinality, s)
	}
}

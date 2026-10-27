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

// AdjListType is the layout of an edge type's adjacency list, combining
// alignment side (source or destination vertex) with ordering (ordered or not).
// Compare with ==; the numeric values carry no meaning.
type AdjListType uint8

// AdjListType values. The zero value is not a valid layout.
const (
	_ AdjListType = iota
	// UnorderedBySource groups edges by source vertex, source side unordered
	// (COO layout).
	UnorderedBySource
	// UnorderedByDest groups edges by destination vertex, dest side unordered.
	UnorderedByDest
	// OrderedBySource groups edges by source vertex, sorted by source (CSR).
	OrderedBySource
	// OrderedByDest groups edges by destination vertex, sorted by dest (CSC).
	OrderedByDest
)

// alignedBySrc / alignedByDst are the on-disk aligned_by values, shared by
// AlignedBy (write) and AdjListTypeFromOrderedAligned (read).
const (
	alignedBySrc = "src"
	alignedByDst = "dst"
)

// On-disk spellings, shared by String (write) and ParseAdjListType (read) so the
// two cannot drift.
const (
	adjListUnorderedBySourceName = "unordered_by_source"
	adjListUnorderedByDestName   = "unordered_by_dest"
	adjListOrderedBySourceName   = "ordered_by_source"
	adjListOrderedByDestName     = "ordered_by_dest"
)

// String returns the on-disk spelling.
func (a AdjListType) String() string {
	switch a {
	case UnorderedBySource:
		return adjListUnorderedBySourceName
	case UnorderedByDest:
		return adjListUnorderedByDestName
	case OrderedBySource:
		return adjListOrderedBySourceName
	case OrderedByDest:
		return adjListOrderedByDestName
	default:
		return "unknown"
	}
}

// IsOrdered reports whether the layout is ordered.
func (a AdjListType) IsOrdered() bool {
	return a == OrderedBySource || a == OrderedByDest
}

// AlignedBy reports the vertex side ("src" or "dst") the layout is aligned by.
// For unknown values, it returns the empty string and false.
func (a AdjListType) AlignedBy() (side string, ok bool) {
	switch a {
	case UnorderedBySource, OrderedBySource:
		return alignedBySrc, true
	case UnorderedByDest, OrderedByDest:
		return alignedByDst, true
	default:
		return "", false
	}
}

// ParseAdjListType parses the on-disk spelling produced by AdjListType.String.
func ParseAdjListType(s string) (AdjListType, error) {
	switch s {
	case adjListUnorderedBySourceName:
		return UnorderedBySource, nil
	case adjListUnorderedByDestName:
		return UnorderedByDest, nil
	case adjListOrderedBySourceName:
		return OrderedBySource, nil
	case adjListOrderedByDestName:
		return OrderedByDest, nil
	default:
		return 0, fmt.Errorf("%w: %q", ErrInvalidAdjListType, s)
	}
}

// AdjListTypeFromOrderedAligned derives an AdjListType from the legacy
// (ordered, aligned_by) pair used by older yaml fixtures. The aligned argument
// must be "src" or "dst"; any other value returns an error wrapping
// ErrInvalidAdjListType. This is the inverse of (IsOrdered, AlignedBy).
func AdjListTypeFromOrderedAligned(ordered bool, aligned string) (AdjListType, error) {
	switch aligned {
	case alignedBySrc:
		if ordered {
			return OrderedBySource, nil
		}
		return UnorderedBySource, nil
	case alignedByDst:
		if ordered {
			return OrderedByDest, nil
		}
		return UnorderedByDest, nil
	default:
		return 0, fmt.Errorf("%w: aligned_by must be %q or %q, got %q",
			ErrInvalidAdjListType, alignedBySrc, alignedByDst, aligned)
	}
}

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

import (
	"errors"
	"testing"
)

func TestAdjListTypeRoundTrip(t *testing.T) {
	t.Parallel()
	cases := []struct {
		v       AdjListType
		s       string
		ordered bool
		side    string
	}{
		{UnorderedBySource, "unordered_by_source", false, "src"},
		{UnorderedByDest, "unordered_by_dest", false, "dst"},
		{OrderedBySource, "ordered_by_source", true, "src"},
		{OrderedByDest, "ordered_by_dest", true, "dst"},
	}
	for _, c := range cases {
		if c.v.String() != c.s {
			t.Errorf("String() = %q, want %q", c.v.String(), c.s)
		}
		got, err := ParseAdjListType(c.s)
		if err != nil {
			t.Errorf("ParseAdjListType(%q): %v", c.s, err)
			continue
		}
		if got != c.v {
			t.Errorf("ParseAdjListType(%q) = %v, want %v", c.s, got, c.v)
		}
		if c.v.IsOrdered() != c.ordered {
			t.Errorf("%v IsOrdered() = %v, want %v", c.v, c.v.IsOrdered(), c.ordered)
		}
		side, ok := c.v.AlignedBy()
		if !ok || side != c.side {
			t.Errorf("%v AlignedBy() = (%q,%v), want (%q,true)", c.v, side, ok, c.side)
		}
	}
}

func TestAdjListTypeUnknown(t *testing.T) {
	t.Parallel()
	bad := AdjListType(0xff)
	if bad.String() != "unknown" {
		t.Errorf("unknown String = %q", bad.String())
	}
	if bad.IsOrdered() {
		t.Errorf("unknown should not report IsOrdered")
	}
	if side, ok := bad.AlignedBy(); ok || side != "" {
		t.Errorf("unknown AlignedBy = (%q,%v)", side, ok)
	}
}

func TestParseAdjListTypeErrors(t *testing.T) {
	t.Parallel()
	for _, in := range []string{"", "ordered_by_src", "csr", "ordered"} {
		_, err := ParseAdjListType(in)
		if err == nil {
			t.Errorf("ParseAdjListType(%q) succeeded, want error", in)
			continue
		}
		if !errors.Is(err, ErrInvalidAdjListType) {
			t.Errorf("ParseAdjListType(%q) error %v not wrapped", in, err)
		}
	}
}

func TestAdjListTypeFromOrderedAligned(t *testing.T) {
	t.Parallel()
	cases := []struct {
		ordered bool
		aligned string
		want    AdjListType
	}{
		{true, "src", OrderedBySource},
		{true, "dst", OrderedByDest},
		{false, "src", UnorderedBySource},
		{false, "dst", UnorderedByDest},
	}
	for _, c := range cases {
		got, err := AdjListTypeFromOrderedAligned(c.ordered, c.aligned)
		if err != nil {
			t.Errorf("AdjListTypeFromOrderedAligned(%v,%q): %v", c.ordered, c.aligned, err)
			continue
		}
		if got != c.want {
			t.Errorf("AdjListTypeFromOrderedAligned(%v,%q) = %v, want %v", c.ordered, c.aligned, got, c.want)
		}
		// Round-tripping back through (IsOrdered, AlignedBy) must return the same value.
		side, ok := got.AlignedBy()
		if !ok {
			t.Errorf("AlignedBy returned ok=false for %v", got)
			continue
		}
		round, err := AdjListTypeFromOrderedAligned(got.IsOrdered(), side)
		if err != nil || round != got {
			t.Errorf("round-trip failed: round=%v err=%v", round, err)
		}
	}
	if _, err := AdjListTypeFromOrderedAligned(true, "other"); !errors.Is(err, ErrInvalidAdjListType) {
		t.Errorf("expected ErrInvalidAdjListType for bogus aligned, got %v", err)
	}
}

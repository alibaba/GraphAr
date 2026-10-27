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

func TestCardinalityRoundTrip(t *testing.T) {
	t.Parallel()
	for _, c := range []Cardinality{CardinalitySingle, CardinalityList, CardinalitySet} {
		s := c.String()
		got, err := ParseCardinality(s)
		if err != nil {
			t.Errorf("ParseCardinality(%q): %v", s, err)
			continue
		}
		if got != c {
			t.Errorf("round-trip %v: got %v", c, got)
		}
	}
}

func TestCardinalityStringUnknown(t *testing.T) {
	t.Parallel()
	if (Cardinality(0xff)).String() != "unknown" {
		t.Errorf("unknown Cardinality string mismatch")
	}
}

func TestParseCardinalityErrors(t *testing.T) {
	t.Parallel()
	for _, in := range []string{"", "Single", "many", "list "} {
		_, err := ParseCardinality(in)
		if err == nil {
			t.Errorf("ParseCardinality(%q) succeeded, want error", in)
			continue
		}
		if !errors.Is(err, ErrInvalidCardinality) {
			t.Errorf("ParseCardinality(%q) not wrapped: %v", in, err)
		}
	}
}

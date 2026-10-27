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

func mustNewInfoVersion(t *testing.T, version int, names ...string) InfoVersion {
	t.Helper()
	v, err := NewInfoVersion(version, names...)
	if err != nil {
		t.Fatalf("NewInfoVersion(%d, %q) = %v", version, names, err)
	}
	return v
}

func TestNewInfoVersion(t *testing.T) {
	t.Parallel()
	v, err := NewInfoVersion(1)
	if err != nil {
		t.Fatalf("NewInfoVersion(1): %v", err)
	}
	if v.Version() != 1 || len(v.UserDefinedTypes()) != 0 {
		t.Errorf("NewInfoVersion(1) = %+v", v)
	}
	v2, err := NewInfoVersion(1, "geometry", "point")
	if err != nil {
		t.Fatalf("NewInfoVersion(1, ...): %v", err)
	}
	if v2.Version() != 1 || len(v2.UserDefinedTypes()) != 2 ||
		v2.UserDefinedTypes()[0] != "geometry" || v2.UserDefinedTypes()[1] != "point" {
		t.Errorf("NewInfoVersion(1, ...) = %+v", v2)
	}
	// Mutating the input slice must not affect the stored value.
	orig := []string{"geometry"}
	v3, err := NewInfoVersion(1, orig...)
	if err != nil {
		t.Fatalf("NewInfoVersion(1, ...): %v", err)
	}
	orig[0] = "mutated"
	if v3.UserDefinedTypes()[0] != "geometry" {
		t.Errorf("UserDefinedTypes was not defensively copied")
	}
	// The getter must also hand back a copy, not the stored slice.
	got := v3.UserDefinedTypes()
	got[0] = "mutated"
	if v3.UserDefinedTypes()[0] != "geometry" {
		t.Errorf("UserDefinedTypes() leaked the internal slice")
	}
}

func TestNewInfoVersionRejectsUnsupportedVersions(t *testing.T) {
	t.Parallel()
	for _, version := range []int{0, -1, 2, 99} {
		_, err := NewInfoVersion(version)
		if err == nil {
			t.Errorf("NewInfoVersion(%d) succeeded, want error", version)
			continue
		}
		if !errors.Is(err, ErrInvalidVersion) {
			t.Errorf("NewInfoVersion(%d) error %v not wrapped", version, err)
		}
	}
}

func TestInfoVersionString(t *testing.T) {
	t.Parallel()
	cases := []struct {
		v    InfoVersion
		want string
	}{
		{mustNewInfoVersion(t, 1), "gar/v1"},
		{mustNewInfoVersion(t, 1, "geometry"), "gar/v1 (geometry)"},
		{mustNewInfoVersion(t, 1, "geometry", "point"), "gar/v1 (geometry,point)"},
	}
	for _, c := range cases {
		if got := c.v.String(); got != c.want {
			t.Errorf("String() = %q, want %q", got, c.want)
		}
	}
}

func TestInfoVersionEqual(t *testing.T) {
	t.Parallel()
	a := mustNewInfoVersion(t, 1, "g")
	b := mustNewInfoVersion(t, 1, "g")
	if !a.Equal(b) {
		t.Error("identical InfoVersions not equal")
	}
	// Only version 1 is supported, so build the differing version directly.
	if a.Equal(InfoVersion{version: 2, userDefinedTypes: []string{"g"}}) {
		t.Error("different version reported equal")
	}
	if a.Equal(mustNewInfoVersion(t, 1)) {
		t.Error("different type lists reported equal")
	}
	if a.Equal(mustNewInfoVersion(t, 1, "h")) {
		t.Error("different type names reported equal")
	}
}

func TestParseInfoVersionOK(t *testing.T) {
	t.Parallel()
	cases := []struct {
		in   string
		want InfoVersion
	}{
		{"gar/v1", mustNewInfoVersion(t, 1)},
		{"gar/v1  ", mustNewInfoVersion(t, 1)},
		{"gar/v1 (geometry)", mustNewInfoVersion(t, 1, "geometry")},
		{"gar/v1 (geometry, point)", mustNewInfoVersion(t, 1, "geometry", "point")},
		{"gar/v1(a,b,c)", mustNewInfoVersion(t, 1, "a", "b", "c")},
		// Trailing text is ignored; a list is read only when '(' directly
		// follows the version number.
		{"gar/v1abc", mustNewInfoVersion(t, 1)},
		{"gar/v1 xyz", mustNewInfoVersion(t, 1)},
		{"gar/v1 xyz(a,b)", mustNewInfoVersion(t, 1)},
		{"gar/v1 (foo)trailing", mustNewInfoVersion(t, 1, "foo")},
		{"gar/v1 (", mustNewInfoVersion(t, 1)},
		{"gar/v1 (foo", mustNewInfoVersion(t, 1)},
		{"gar/v1 ()", mustNewInfoVersion(t, 1)},
		{"gar/v1 (a,,b)", mustNewInfoVersion(t, 1, "a", "b")},
	}
	for _, c := range cases {
		got, err := ParseInfoVersion(c.in)
		if err != nil {
			t.Errorf("ParseInfoVersion(%q): %v", c.in, err)
			continue
		}
		if !got.Equal(c.want) {
			t.Errorf("ParseInfoVersion(%q) = %+v, want %+v", c.in, got, c.want)
		}
	}
}

func TestParseInfoVersionErrors(t *testing.T) {
	t.Parallel()
	cases := []string{
		"",
		"   ",
		"gar/v",
		"gar/vabc",
		"gar/v-1",
		"gar/v99999999999999999999999999999999999999999999999999", // overflows int
		"foo/v1",
		"v1",
		" gar/v1", // leading whitespace is not part of the format
		"gar/v0",  // unsupported version
		"gar/v2",  // unsupported version
		"gar/v99", // unsupported version
	}
	for _, in := range cases {
		_, err := ParseInfoVersion(in)
		if err == nil {
			t.Errorf("ParseInfoVersion(%q) succeeded, want error", in)
			continue
		}
		if !errors.Is(err, ErrInvalidVersion) {
			t.Errorf("ParseInfoVersion(%q) error %v not wrapped", in, err)
		}
	}
}

func TestParseInfoVersionRoundTrip(t *testing.T) {
	t.Parallel()
	for _, in := range []string{"gar/v1", "gar/v1 (geometry,point)"} {
		v, err := ParseInfoVersion(in)
		if err != nil {
			t.Fatalf("ParseInfoVersion(%q): %v", in, err)
		}
		if v.String() != in {
			t.Errorf("round-trip failed: ParseInfoVersion(%q).String() = %q", in, v.String())
		}
	}
}

func TestDefaultVersionConstant(t *testing.T) {
	t.Parallel()
	if DefaultVersion != 1 {
		t.Errorf("DefaultVersion changed to %d; cross-language fixtures still ship gar/v1", DefaultVersion)
	}
}

// TestInfoVersionCloneIndependent guards the immutability contract: Clone must
// return a version whose UserDefinedTypes slice is independent of the source,
// so a value handed out by an Info's Version() getter cannot mutate stored
// state.
func TestInfoVersionCloneIndependent(t *testing.T) {
	t.Parallel()
	orig := mustNewInfoVersion(t, 1, "geometry", "point")
	clone := orig.Clone()
	if !clone.Equal(orig) {
		t.Fatalf("Clone not equal to source: %+v vs %+v", clone, orig)
	}
	clone.userDefinedTypes[0] = "hacked"
	if orig.userDefinedTypes[0] != "geometry" {
		t.Errorf("mutating clone leaked into source: %v", orig.userDefinedTypes)
	}
	// Empty case must not panic and stays independent.
	empty := mustNewInfoVersion(t, 1)
	if got := empty.Clone(); len(got.userDefinedTypes) != 0 || got.version != 1 {
		t.Errorf("empty clone wrong: %+v", got)
	}
}

func TestInfoVersionValidate(t *testing.T) {
	t.Parallel()
	valid := []InfoVersion{
		mustNewInfoVersion(t, 1),
		mustNewInfoVersion(t, 1, "geometry", "point"),
	}
	for _, v := range valid {
		if err := v.Validate(); err != nil {
			t.Errorf("Validate(%q) = %v, want nil", v, err)
		}
	}

	invalid := []struct {
		name string
		v    InfoVersion
	}{
		{"zero version", InfoVersion{version: 0}},
		{"negative version", InfoVersion{version: -1}},
		{"unsupported version", InfoVersion{version: 2}},
		{"name with comma", InfoVersion{version: 1, userDefinedTypes: []string{"my,type"}}},
		{"name with surrounding space", InfoVersion{version: 1, userDefinedTypes: []string{" geometry "}}},
		{"empty name", InfoVersion{version: 1, userDefinedTypes: []string{""}}},
	}
	for _, c := range invalid {
		if err := c.v.Validate(); !errors.Is(err, ErrInvalidVersion) {
			t.Errorf("Validate(%s) = %v, want ErrInvalidVersion", c.name, err)
		}
	}
}

// TestInfoVersionCommaNameRejected pins the round-trip hazard: a name with a
// comma would split into two on ParseInfoVersion, so Validate must reject it
// before it reaches String.
func TestInfoVersionCommaNameRejected(t *testing.T) {
	t.Parallel()
	v := InfoVersion{version: 1, userDefinedTypes: []string{"a,b"}}
	if err := v.Validate(); !errors.Is(err, ErrInvalidVersion) {
		t.Fatalf("Validate = %v, want ErrInvalidVersion", err)
	}
	// A validated (single, clean) name must survive String -> ParseInfoVersion.
	ok := mustNewInfoVersion(t, 1, "geometry")
	if err := ok.Validate(); err != nil {
		t.Fatalf("Validate = %v, want nil", err)
	}
	back, err := ParseInfoVersion(ok.String())
	if err != nil || !back.Equal(ok) {
		t.Errorf("round-trip %q -> %+v (err %v)", ok.String(), back, err)
	}
}

func TestInfoVersionCheckType(t *testing.T) {
	t.Parallel()
	v := mustNewInfoVersion(t, 1, "geometry")
	cases := []struct {
		typeStr string
		want    bool
	}{
		{"int32", true},    // built-in type of v1
		{"string", true},   // built-in type of v1
		{"geometry", true}, // user-defined type
		{"point", false},   // neither built-in nor user-defined
	}
	for _, c := range cases {
		if got := v.CheckType(c.typeStr); got != c.want {
			t.Errorf("CheckType(%q) = %v, want %v", c.typeStr, got, c.want)
		}
	}
	if got := (InfoVersion{}).CheckType("int32"); got {
		t.Error("zero InfoVersion must not report built-in types")
	}
}

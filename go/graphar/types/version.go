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
	"fmt"
	"slices"
	"strconv"
	"strings"
)

// DefaultVersion is the version assumed for Info objects that do not specify
// one explicitly. It is the on-disk default for the GraphAr format.
const DefaultVersion = 1

// versionPrefix is the on-disk marker every version string starts with. Shared
// by String (write) and ParseInfoVersion (read) so the two never drift.
const versionPrefix = "gar/v"

// InfoVersion is the on-disk schema version of an Info document.
//
// The string form is "gar/vN" (e.g. "gar/v1"). An optional user-defined type
// list may follow in parentheses: "gar/v1 (foo,bar)". User-defined types are
// recorded in declaration order and duplicates are kept; surrounding
// whitespace on each entry is trimmed on parse (so "gar/v1 (a, b)" round-trips
// as "gar/v1 (a,b)"). The zero value is not usable; build one with
// NewInfoVersion or ParseInfoVersion.
type InfoVersion struct {
	version          int
	userDefinedTypes []string
}

// version2types lists the built-in property type names each schema version
// supports, kept in step with the other GraphAr implementations.
var version2types = map[int][]string{
	1: {"bool", "int32", "int64", "float", "double", "string"},
}

// NewInfoVersion returns an InfoVersion for a supported schema version,
// copying userDefinedTypes defensively.
func NewInfoVersion(version int, userDefinedTypes ...string) (InfoVersion, error) {
	if version <= 0 {
		return InfoVersion{}, fmt.Errorf("%w: version must be positive, got %d", ErrInvalidVersion, version)
	}
	if _, ok := version2types[version]; !ok {
		return InfoVersion{}, fmt.Errorf("%w: unsupported version %d", ErrInvalidVersion, version)
	}
	if len(userDefinedTypes) == 0 {
		return InfoVersion{version: version}, nil
	}
	return InfoVersion{version: version, userDefinedTypes: slices.Clone(userDefinedTypes)}, nil
}

// Version returns the schema version number.
func (v InfoVersion) Version() int { return v.version }

// UserDefinedTypes returns a copy of the user-defined type names in declaration
// order.
func (v InfoVersion) UserDefinedTypes() []string {
	return slices.Clone(v.userDefinedTypes)
}

// CheckType reports whether typeStr is a property type this version supports,
// either a built-in type of the version or one of its user-defined types.
func (v InfoVersion) CheckType(typeStr string) bool {
	if supported, ok := version2types[v.version]; ok && slices.Contains(supported, typeStr) {
		return true
	}
	return slices.Contains(v.userDefinedTypes, typeStr)
}

// String returns the on-disk spelling.
func (v InfoVersion) String() string {
	if len(v.userDefinedTypes) == 0 {
		return fmt.Sprintf("%s%d", versionPrefix, v.version)
	}
	return fmt.Sprintf("%s%d (%s)", versionPrefix, v.version, strings.Join(v.userDefinedTypes, ","))
}

// Clone returns a deep copy of v, independent of the source's user-defined type
// slice; getters that hand out a stored InfoVersion should return v.Clone().
func (v InfoVersion) Clone() InfoVersion {
	if len(v.userDefinedTypes) == 0 {
		return InfoVersion{version: v.version}
	}
	return InfoVersion{version: v.version, userDefinedTypes: slices.Clone(v.userDefinedTypes)}
}

// Validate reports whether v is well-formed: a positive version number and
// user-defined type names that round-trip through String / ParseInfoVersion.
func (v InfoVersion) Validate() error {
	if v.version <= 0 {
		return fmt.Errorf("%w: version must be positive, got %d", ErrInvalidVersion, v.version)
	}
	if _, ok := version2types[v.version]; !ok {
		return fmt.Errorf("%w: unsupported version %d", ErrInvalidVersion, v.version)
	}
	for _, name := range v.userDefinedTypes {
		if err := validateTypeName(name); err != nil {
			return fmt.Errorf("%w: %s", ErrInvalidVersion, err)
		}
	}
	return nil
}

// validateTypeName reports why name cannot survive a String / ParseInfoVersion
// round trip: it must be non-empty, carry no surrounding whitespace and avoid
// the delimiters the version string and list syntax rely on (, ( ) < >).
func validateTypeName(name string) error {
	switch {
	case name == "":
		return errors.New("user-defined type name must not be empty")
	case strings.TrimSpace(name) != name:
		return fmt.Errorf("user-defined type name %q has leading or trailing whitespace", name)
	case strings.ContainsAny(name, ",()<>"):
		return fmt.Errorf("user-defined type name %q must not contain any of , ( ) < >", name)
	}
	return nil
}

// Equal reports whether two InfoVersions are equivalent.
func (v InfoVersion) Equal(other InfoVersion) bool {
	return v.version == other.version &&
		slices.Equal(v.userDefinedTypes, other.userDefinedTypes)
}

// ParseInfoVersion parses "gar/vN" or "gar/vN (t1,t2,...)".
//
// The version must be one the package supports. The type list is recognized
// only when '(' follows the version number directly, with spaces allowed in
// between; trailing text elsewhere is ignored. Entries are trimmed, blank
// entries are skipped, and an unterminated list is dropped.
func ParseInfoVersion(s string) (InfoVersion, error) {
	if !strings.HasPrefix(s, versionPrefix) {
		return InfoVersion{}, fmt.Errorf("%w: must start with %q, got %q", ErrInvalidVersion, versionPrefix, s)
	}
	rest := s[len(versionPrefix):]

	end := 0
	for end < len(rest) && rest[end] >= '0' && rest[end] <= '9' {
		end++
	}
	if end == 0 {
		return InfoVersion{}, fmt.Errorf("%w: bad version number in %q", ErrInvalidVersion, s)
	}
	n, err := strconv.Atoi(rest[:end])
	if err != nil {
		return InfoVersion{}, fmt.Errorf("%w: bad version number in %q", ErrInvalidVersion, s)
	}
	if _, ok := version2types[n]; !ok {
		return InfoVersion{}, fmt.Errorf("%w: unsupported version %d in %q", ErrInvalidVersion, n, s)
	}

	out := InfoVersion{version: n}
	// "(t1,t2,...)" list: only when '(' starts right after the digits, with at
	// most spaces in between. A '(' without a later ')' is ignored.
	if lparen := strings.IndexByte(rest, '('); lparen >= 0 && strings.Trim(rest[end:lparen], " ") == "" {
		if rparen := strings.LastIndexByte(rest, ')'); rparen > lparen {
			for _, p := range strings.Split(rest[lparen+1:rparen], ",") {
				if t := strings.TrimSpace(p); t != "" {
					out.userDefinedTypes = append(out.userDefinedTypes, t)
				}
			}
		}
	}
	return out, nil
}

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

func TestFileTypeRoundTrip(t *testing.T) {
	t.Parallel()
	for _, ft := range []FileType{FileTypeCSV, FileTypeParquet, FileTypeORC, FileTypeJSON} {
		s := ft.String()
		got, err := ParseFileType(s)
		if err != nil {
			t.Errorf("ParseFileType(%q): %v", s, err)
			continue
		}
		if got != ft {
			t.Errorf("round-trip %v: got %v", ft, got)
		}
	}
}

func TestFileTypeIsZeroAndStringUnset(t *testing.T) {
	t.Parallel()
	var f FileType
	if !f.IsZero() {
		t.Error("zero FileType should report IsZero")
	}
	if got := f.String(); got != "unset" {
		t.Errorf("zero FileType String() = %q, want unset", got)
	}
	if _, err := ParseFileType("unset"); err == nil {
		t.Error("ParseFileType(\"unset\") should be rejected")
	}
	if FileTypeCSV.IsZero() {
		t.Error("FileTypeCSV must not report IsZero")
	}
}

func TestFileTypeStringUnknown(t *testing.T) {
	t.Parallel()
	if (FileType(0xff)).String() != "unknown" {
		t.Errorf("unknown FileType should stringify to %q", "unknown")
	}
}

func TestParseFileTypeErrors(t *testing.T) {
	t.Parallel()
	for _, in := range []string{"", "CSV", "tsv", "parquette"} {
		_, err := ParseFileType(in)
		if err == nil {
			t.Errorf("ParseFileType(%q) succeeded, want error", in)
			continue
		}
		if !errors.Is(err, ErrInvalidFileType) {
			t.Errorf("ParseFileType(%q) error %v not wrapped by ErrInvalidFileType", in, err)
		}
	}
}

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

// FileType is the on-disk physical format of a property group or adjacency
// list: csv / parquet / orc / json. (json parses but is rejected at validation
// time; see FileTypeJSON.)
type FileType uint8

// FileType values. Zero value is unset; constructed values are non-zero so
// callers can detect a forgotten assignment via IsZero().
const (
	// fileTypeUnset is the zero / unset sentinel. Not exported because it
	// is never a valid on-disk format.
	fileTypeUnset FileType = iota
	// FileTypeCSV is the comma-separated values text format.
	FileTypeCSV
	// FileTypeParquet is the Apache Parquet columnar format.
	FileTypeParquet
	// FileTypeORC is the Apache ORC columnar format.
	FileTypeORC
	// FileTypeJSON is the JSON text format. Parsing accepts it only for
	// read-only fixture compatibility; it is not a supported output format.
	FileTypeJSON
)

// On-disk spellings, shared by String (write) and ParseFileType (read) so the
// two cannot drift.
const (
	fileTypeCSVName     = "csv"
	fileTypeParquetName = "parquet"
	fileTypeORCName     = "orc"
	fileTypeJSONName    = "json"
)

// IsZero reports whether f is the zero (unset) FileType.
func (f FileType) IsZero() bool { return f == fileTypeUnset }

// String returns the on-disk spelling. The unset zero value stringifies to
// "unset" (not a valid file type).
func (f FileType) String() string {
	switch f {
	case fileTypeUnset:
		return "unset"
	case FileTypeCSV:
		return fileTypeCSVName
	case FileTypeParquet:
		return fileTypeParquetName
	case FileTypeORC:
		return fileTypeORCName
	case FileTypeJSON:
		return fileTypeJSONName
	default:
		return "unknown"
	}
}

// ParseFileType parses the on-disk spelling produced by FileType.String.
func ParseFileType(s string) (FileType, error) {
	switch s {
	case fileTypeCSVName:
		return FileTypeCSV, nil
	case fileTypeParquetName:
		return FileTypeParquet, nil
	case fileTypeORCName:
		return FileTypeORC, nil
	case fileTypeJSONName:
		return FileTypeJSON, nil
	default:
		return fileTypeUnset, fmt.Errorf("%w: %q", ErrInvalidFileType, s)
	}
}

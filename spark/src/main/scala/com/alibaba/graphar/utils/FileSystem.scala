/** Copyright 2022 Alibaba Group Holding Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.alibaba.graphar.utils

import java.net.URI
import org.apache.spark.sql.SparkSession
import org.apache.spark.sql.DataFrame
import org.apache.hadoop.fs

/** Helper object to write dataframe to chunk files */
object FileSystem {
  private def renameSparkGeneratedFiles(spark: SparkSession, filePrefix: String, startChunkIndex: Int): Unit = {
    val sc = spark.sparkContext
    val file_system = fs.FileSystem.get(new URI(filePrefix), spark.sparkContext.hadoopConfiguration)
    val path_pattern = new fs.Path(filePrefix + "part*")
    val files = file_system.globStatus(path_pattern)
    for (i <- 0 until files.length) {
      val file_name = files(i).getPath.getName
      val new_file_name = "chunk" + (i + startChunkIndex).toString
      val destPath = new fs.Path(filePrefix + new_file_name)
      if (file_system.isFile(destPath)) {
        // if chunk file already exists, overwrite it
        file_system.delete(destPath)
      }
      file_system.rename(new fs.Path(filePrefix + file_name), destPath)
    }
  }

  /** Write input dataframe to output path with certain file format.
   *
   * @param dataframe DataFrame to write out.
   * @param fileType output file format type, the value could be csv|parquet|orc.
   * @param outputPrefix output path prefix.
   * @param startChunkIndex the start index of chunk.
   *
   */
  def writeDataFrame(dataFrame: DataFrame, fileType: String, outputPrefix: String, startChunkIndex: Int = 0): Unit = {
    val spark = dataFrame.sparkSession
    spark.conf.set("mapreduce.fileoutputcommitter.marksuccessfuljobs", "false")
    spark.conf.set("parquet.enable.summary-metadata", "false")
    dataFrame.write.mode("append").format(fileType).save(outputPrefix)
    renameSparkGeneratedFiles(spark, outputPrefix, startChunkIndex)
  }
}

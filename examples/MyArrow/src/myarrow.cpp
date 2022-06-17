// example from
// https://github.com/apache/arrow/blob/master/cpp/examples/minimal_build/example.cc

#include <arrow/csv/options.h>          // for ConvertOptions, ParseOptions
#include <arrow/csv/reader.h>           // for TableReader
#include <arrow/io/file.h>              // for FileOutputStream, ReadableFile
#include <arrow/io/type_fwd.h>          // for default_io_context
#include <arrow/ipc/writer.h>           // for MakeFileWriter, RecordBatchWr...
#include <arrow/pretty_print.h>         // for PrettyPrint
#include <arrow/result.h>               // for ARROW_ASSIGN_OR_RAISE
#include <arrow/status.h>               // for Status, ARROW_RETURN_NOT_OK
#include <arrow/table.h>                // for Table
#include <arrow/util/string_builder.h>  // for operator<<
#include <iostream>                     // for operator<<, endl, basic_ostream
#include <memory>                       // for __shared_ptr_access, shared_ptr

using arrow::Status;

namespace {

Status RunMain(int argc, char** argv) {
  const char* csv_filename = "test.csv";
  const char* arrow_filename = "test.arrow";

  std::cerr << "* Reading CSV file '" << csv_filename << "' into table"
            << std::endl;
  ARROW_ASSIGN_OR_RAISE(
      auto input_file, arrow::io::ReadableFile::Open(csv_filename));
  ARROW_ASSIGN_OR_RAISE(
      auto csv_reader,
      arrow::csv::TableReader::Make(
          arrow::io::default_io_context(),
          input_file,
          arrow::csv::ReadOptions::Defaults(),
          arrow::csv::ParseOptions::Defaults(),
          arrow::csv::ConvertOptions::Defaults()));
  ARROW_ASSIGN_OR_RAISE(auto table, csv_reader->Read());

  std::cerr << "* Read table:" << std::endl;
  ARROW_RETURN_NOT_OK(arrow::PrettyPrint(*table, {}, &std::cerr));

  std::cerr << "* Writing table into Arrow IPC file '" << arrow_filename << "'"
            << std::endl;
  ARROW_ASSIGN_OR_RAISE(
      auto output_file, arrow::io::FileOutputStream::Open(arrow_filename));
  ARROW_ASSIGN_OR_RAISE(
      auto batch_writer,
      arrow::ipc::MakeFileWriter(output_file, table->schema()));
  ARROW_RETURN_NOT_OK(batch_writer->WriteTable(*table));
  ARROW_RETURN_NOT_OK(batch_writer->Close());

  return Status::OK();
}

}  // namespace

int main(int argc, char** argv) {
  Status st = RunMain(argc, argv);
  if (!st.ok()) {
    std::cerr << st << std::endl;
    return 1;
  }
  return 0;
}

arx_build_lib = static_library(
  'arx-build',
  project_src_files,
  include_directories : inc,
  install : false,
  dependencies : deps)

TESTS_PATH = PROJECT_PATH + '/tests/unittests'

test_suite = [
  ['error', files(TESTS_PATH + '/test-error.cpp')],
  ['lexer', files(TESTS_PATH + '/test-lexer.cpp')],
  ['parser',files(TESTS_PATH + '/test-parser.cpp')],
  ['utils', files(TESTS_PATH + '/test-utils.cpp')],
  ['input', files(TESTS_PATH + '/test-io.cpp')],
  ['ast-to-object', files(TESTS_PATH + '/codegen/test-ast-to-object.cpp')],
  ['ast-to-stdout', files(TESTS_PATH + '/codegen/test-ast-to-stdout.cpp')],
  ['ast-to-llvm-ir', files(TESTS_PATH + '/codegen/test-ast-to-llvm-ir.cpp')],
]

foreach test_item : test_suite
    test_name = test_item[0]
    test_src_files = test_item[1] + files(TESTS_PATH + '/main.cpp')

    executable_name_suffix = test_name + '_tests'
    test_executable = executable(
      'arx_' + executable_name_suffix,
      test_src_files,
      include_directories : inc,
      dependencies : deps,
      link_whole: arx_build_lib)

    test(
      executable_name_suffix,
      test_executable,
      workdir : meson.source_root())
endforeach

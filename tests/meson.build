extra_flags = [
  '-O1',
  '-fno-omit-frame-pointer',
  '-fprofile-instr-generate',
  '-fcoverage-mapping'
]

add_global_arguments(extra_flags, language: 'cpp')
add_global_arguments(extra_flags, language: 'c')

arx_build_lib = static_library(
  'arx-build',
  project_src_files,
  include_directories : inc,
  install : false,
  dependencies : deps)

test_suite = [
  ['codegen', files(PROJECT_PATH + '/tests/test_codegen.cpp')],
  ['error', files(PROJECT_PATH + '/tests/test_error.cpp')],
  ['lexer', files(PROJECT_PATH + '/tests/test_lexer.cpp')],
  ['parser', files(PROJECT_PATH + '/tests/test_parser.cpp')],
  ['utils', files(PROJECT_PATH + '/tests/test_utils.cpp')],
]

foreach test_item : test_suite
    test_name = test_item[0]
    test_src_files = test_item[1] + files(PROJECT_PATH + '/tests/main.cpp')

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

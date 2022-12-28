project('arx', 'cpp', 'c',
  license : 'Apache-2.0',
  version : '1.5.0',  # semantic-release
  default_options : [
    'warning_level=3',
    'cpp_std=c++20',
  ]
)

PROJECT_PATH = meson.source_root()

cxx = meson.get_compiler('cpp')

deps = [
  dependency('arrow'),
  dependency('llvm', version : '>=14.0.0'),
  dependency('CLI11'),
  dependency('threads'),
  dependency('glog'),
]

inc = include_directories('./src')

project_src_files = files(
  # note: note working yet
  # PROJECT_PATH + '/src/codegen/ast-to-llvm.cpp',
  PROJECT_PATH + '/src/codegen/ast-to-object.cpp',
  PROJECT_PATH + '/src/error.cpp',
  PROJECT_PATH + '/src/io.cpp',
  PROJECT_PATH + '/src/lexer.cpp',
  PROJECT_PATH + '/src/parser.cpp',
  PROJECT_PATH + '/src/utils.cpp',
)

gtest_dep = dependency('gtest', main : true, required: get_option('dev'))
gmock_dep = dependency('gmock', required: get_option('dev'))

if get_option('dev').enabled()
  deps += [gtest_dep, gmock_dep]
  subdir('tests')
endif

clangtidy = find_program('clang-tidy', required: get_option('dev'))
if clangtidy.found()
  run_target(
    'clang-tidy',
    command: [
      PROJECT_PATH + '/scripts/run-clang-tidy.sh',
    ])
endif

arx_exe = executable(
  'arx',
  project_src_files + files(PROJECT_PATH + '/src/main.cpp'),
  dependencies : deps,
  include_directories : inc,
  install : true)

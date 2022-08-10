project('arx', 'cpp', 'c',
  license : 'Apache-2.0',
  version : '1.4.0',  # semantic-release
  default_options : [
    'warning_level=3',
    'cpp_std=c++17',
  ]
)

PROJECT_PATH = meson.source_root()

cxx = meson.get_compiler('cpp')

deps = [
  dependency('arrow'),
  dependency('llvm', version : '>=13.0.0'),
  dependency('CLI11'),
  dependency('glib-2.0'),
  dependency('glog'),
  dependency('threads'),
]

inc = include_directories('./arx/include')

project_src_files = files(
  PROJECT_PATH + '/arx/src/input.cpp',
  PROJECT_PATH + '/arx/src/lexer.cpp',
  PROJECT_PATH + '/arx/src/parser.cpp',
  PROJECT_PATH + '/arx/src/error.cpp',
  PROJECT_PATH + '/arx/src/codegen.cpp',
  PROJECT_PATH + '/arx/src/settings.cpp',
  PROJECT_PATH + '/arx/src/utils.cpp',
)

gtest_dep = dependency('gtest', main : true, required: get_option('dev'))
gmock_dep = dependency('gmock', required: get_option('dev'))

if get_option('dev').enabled()
  deps += [gtest_dep, gmock_dep]
  subdir('tests')
else
  project_src_files += files(PROJECT_PATH + '/arx/src/main.cpp')
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
  project_src_files,
  dependencies : deps,
  include_directories : inc,
  install : true)

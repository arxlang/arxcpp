project('arx', 'cpp', 'c',
  license : 'Apache-2.0',
  version : '1.6.0',  # semantic-release
  default_options : [
    'warning_level=everything',
    'cpp_std=c++20',
  ]
)

add_global_arguments(
  '-Wpedantic',
  '-Wno-c++98-compat-pedantic',
  '-Wno-padded',
  '-Wno-missing-prototypes',
  '-Wshadow',
  '-Wnon-virtual-dtor',
  '-Wcast-align',
  '-Wunused',
  '-Woverloaded-virtual',
  '-Wconversion',
  '-Wsign-conversion',
  '-Wdouble-promotion',
  '-Wformat=2',
  '-Wimplicit-fallthrough',
  '-Wsuggest-override',
  '-Wnull-dereference',
  '-Wold-style-cast',
  language : 'cpp'
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
  PROJECT_PATH + '/src/codegen/ast-to-llvm-ir.cpp',
  PROJECT_PATH + '/src/codegen/ast-to-object.cpp',
  PROJECT_PATH + '/src/codegen/ast-to-stdout.cpp',
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

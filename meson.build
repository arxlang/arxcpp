project('arx', 'cpp', 'c',
  license : 'Apache-2.0',
  version : '1.6.0',  # semantic-release
  default_options : [
    #'warning_level=everything',
    'warning_level=0',
    'cpp_std=c++20',
  ]
)

add_global_arguments(
  '-Wpedantic',
  # '-Wno-c++98-compat-pedantic',
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

llvm_modules = [
  'core',
  'executionengine',
  'object',
  'orcjit',
  'support',
  'native',
]

deps = [
  dependency('arrow'),
  dependency('arrow-glib'),
  dependency('llvm', version : '>=15.0.0', modules : llvm_modules),
  dependency('CLI11'),
  dependency('threads'),
  dependency('glog'),
]

inc = include_directories('./src')

SRC_PATH = PROJECT_PATH + '/src'

project_src_files = files(
  SRC_PATH + '/codegen/arx-llvm.cpp',
  SRC_PATH + '/codegen/ast-to-llvm-ir.cpp',
  SRC_PATH + '/codegen/ast-to-object.cpp',
  SRC_PATH + '/codegen/ast-to-stdout.cpp',
  SRC_PATH + '/error.cpp',
  SRC_PATH + '/io.cpp',
  SRC_PATH + '/lexer.cpp',
  SRC_PATH + '/parser.cpp',
  SRC_PATH + '/utils.cpp',
)

gtest_dep = dependency('gtest', main : true, required: get_option('dev'))
gmock_dep = dependency('gmock', required: get_option('dev'))

if get_option('dev').enabled()
  deps += [gtest_dep, gmock_dep]
  subdir('tests/unittests')
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

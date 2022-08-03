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

inc = include_directories('arx/include')

project_files = [
  PROJECT_PATH + '/arx/src/input.cpp',
  PROJECT_PATH + '/arx/src/lexer.cpp',
  PROJECT_PATH + '/arx/src/parser.cpp',
  PROJECT_PATH + '/arx/src/error.cpp',
  PROJECT_PATH + '/arx/src/codegen.cpp',
  PROJECT_PATH + '/arx/src/settings.cpp',
  PROJECT_PATH + '/arx/src/utils.cpp',
]

src_files = project_files

gtest_dep = dependency('gtest', required: get_option('tests'))
if gtest_dep.found()
    deps += [gtest_dep]
  src_files += [
    PROJECT_PATH + '/tests/main.cpp',
    PROJECT_PATH + '/tests/test_codegen.cpp',
    PROJECT_PATH + '/tests/test_error.cpp',
    PROJECT_PATH + '/tests/test_lexer.cpp',
    PROJECT_PATH + '/tests/test_parser.cpp',
    PROJECT_PATH + '/tests/test_utils.cpp'
  ]
else
  src_files += [PROJECT_PATH + '/src/main.cpp']
endif

clangtidy = find_program('clang-tidy', required: false)
if clangtidy.found()
    run_target(
        'tidy',
        command: [
            clangtidy,
            '--fix-errors',
            '-p', meson.build_root()
        ] + project_files)
endif

exe = executable(
  'arx',
  src_files,
  dependencies : deps,
  include_directories : inc,
  install : true)

test('basic', exe)

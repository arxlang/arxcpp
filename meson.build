project('arx', 'cpp', 'c',
  license : 'Apache-2.0',
  version : '1.4.0',  # semantic-release
  default_options : [
    'warning_level=3',
    'cpp_std=c++17',
  ]
)

cxx = meson.get_compiler('cpp')

deps = [
  dependency('arrow'),
  dependency('llvm', version : '>=13.0.0'),
  dependency('CLI11'),
  dependency('glog'),
]

inc = include_directories('arx/include')

sources = [
  'arx/src/input.cpp',
  'arx/src/lexer.cpp',
  'arx/src/parser.cpp',
  'arx/src/error.cpp',
  'arx/src/codegen.cpp',
  'arx/src/settings.cpp',
  'arx/src/utils.cpp',
]

if get_option('tests').enabled()
  deps += [dependency('gtest')]
  sources += [
    'tests/main.cpp',
    'tests/test_codegen.cpp',
    'tests/test_error.cpp',
    'tests/test_lexer.cpp',
    'tests/test_parser.cpp',
    'tests/test_utils.cpp'
  ]
else
  sources += ['src/main.cpp']
endif

exe = executable(
  'arx',
  sources,
  dependencies : deps,
  include_directories : inc,
  install : true)

test('basic', exe)

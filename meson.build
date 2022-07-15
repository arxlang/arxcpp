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

exe = executable(
  'arx',
  'arx/src/main.cpp',
  'arx/src/input.cpp',
  'arx/src/lexer.cpp',
  'arx/src/parser.cpp',
  'arx/src/error.cpp',
  'arx/src/codegen.cpp',
  'arx/src/settings.cpp',
  'arx/src/utils.cpp',
  dependencies : deps,
  include_directories : inc,
  install : true)

test('basic', exe)

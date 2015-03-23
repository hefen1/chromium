{
  'TOOLS': ['newlib', 'glibc', 'pnacl', 'linux', 'mac', 'clang-newlib'],
  'TARGETS': [
    {
      'NAME' : 'pi_generator',
      'TYPE' : 'main',
      'SOURCES' : ['pi_generator.cc'],
      'DEPS': ['ppapi_simple', 'nacl_io', 'ppapi_cpp'],
      'LIBS': ['ppapi_simple', 'nacl_io', 'ppapi_cpp', 'ppapi', 'pthread']
    }
  ],
  'DATA': [
    'example.js',
  ],
  'DEST': 'examples/demo',
  'NAME': 'pi_generator',
  'TITLE': 'Monte Carlo Estimate for Pi',
  'GROUP': 'Demo'
}

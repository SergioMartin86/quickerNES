# Getting core configurations
subdir('quickNES')
subdir('quickerNES')

# Tool Configuration

 toolDependency = declare_dependency(
  include_directories : include_directories(['.', '../extern']),
  sources             : [ 
   '../extern/jaffarCommon/extern/metrohash128/metrohash128.cpp',
   '../extern/jaffarCommon/extern/xdelta3/xdelta3.c',
   ]
 )

# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\pChat_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\pChat_autogen.dir\\ParseCache.txt"
  "pChat_autogen"
  )
endif()

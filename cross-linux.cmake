# this one is important
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_PLATFORM Linux)
#this one not so much
set(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
set(CMAKE_C_COMPILER $ENV{CC})
set(CMAKE_CXX_COMPILER $ENV{CXX})
#set(CMAKE_C_COMPILER_ID GNU)
#set(CMAKE_CXX_COMPILER_ID GNU)

#CMAKE_FORCE_C_COMPILER($ENV{CC} GNU)    # mine
#CMAKE_FORCE_CXX_COMPILER($ENV{CXX} GNU) # mine

# where is the target environment
set(CMAKE_INSTALL_PREFIX     $ENV{CONDA_PREFIX} ) # mine
set(CMAKE_PREFIX_PATH        $ENV{CONDA_PREFIX} ) # mine
set(CMAKE_SYSROOT            $ENV{CONDA_PREFIX}/$ENV{HOST}/sysroot ) # mine
set(CMAKE_FIND_ROOT_PATH     $ENV{CONDA_PREFIX} $ENV{CONDA_PREFIX}/$ENV{HOST}/sysroot) # modified
#set(CMAKE_LD                 $ENV{CONDA_PREFIX}/$ENV{HOST}/bin/ld ) # mine


# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# god-awful hack because it seems to not run correct tests to determine this:
set(__CHAR_UNSIGNED___EXITCODE 1)

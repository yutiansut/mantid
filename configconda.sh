#!/usr/bin/bash
# https://docs.conda.io/projects/conda-build/en/latest/resources/compiler-tools.html
SRCDIR=$HOME/code/mantid-conda-attempt/
cd $SRCDIR
echo "CONDA_PREFIX=$CONDA_PREFIX"

ls -lah ~/;rm -rf build ;mkdir build; cd build ; mkdir GL-includes ;cp -R /usr/include/GL GL-includes/

cmake -G Ninja \
      -DUSE_SYSTEM_EIGEN=on -DUSE_CXX98_ABI=on  -DENABLE_OPENCASCADE=off -DCMAKE_SKIP_INSTALL_RPATH=on \
      -DENABLE_MANTIDPLOT=off -DENABLE_WORKBENCH=off \
      -DUSE_CCACHE=off \
      -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX \
      -DCMAKE_TOOLCHAIN_FILE=../cross-linux.cmake \
      -DOPENGL_opengl_LIBRARY=/usr/lib64/libGL.so.1 -DOPENGL_glu_LIBRARY=/usr/lib64/libGLU.so -DOPENGL_INCLUDE_DIR=$SRCDIR/build/GL-includes -DOPENGL_glx_LIBRARY=/usr/lib64/libGLX.so -DOPENGL_gl_LIBRARY=/usr/lib64/libGL.so \
      $SRCDIR

      #-DCMAKE_MAKE_PROGRAM=$CONDA_PREFIX/bin/ninja \

      #-DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX -DCMAKE_PREFIX_PATH=$CONDA_PREFIX \
      #-DCMAKE_SYSROOT=$CONDA_PREFIX/$HOST/sysroot/ \

      #-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
      #-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
      #-DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \

      #-DCMAKE_PROGRAM_PATH=$CONDA_PREFIX/bin
      #-DPKG_CONFIG_EXECUTABLE=$CONDA_PREFIX/bin/pkg-config \
      #-DHDF5_CXX_LIBRARY_dl=$CONDA_PREFIX/$HOST/sysroot/usr/lib/libdl.so \
      #-DHDF5_CXX_LIBRARY_m=$CONDA_PREFIX/$HOST/sysroot/usr/lib/libm.so \
      #-DHDF5_CXX_LIBRARY_pthread=$CONDA_PREFIX/$HOST/sysroot/usr/lib/libpthread.so \
      #-DHDF5_CXX_LIBRARY_rt=$CONDA_PREFIX/$HOST/sysroot/usr/lib/librt.so \

#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
#set(CMAKE_BUILD_TYPE Release)

#set(CMAKE_BINARY_DIR "E:/GitProjects/CppPathtracerRewrite/MScompile/MScompile/build")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin)

#set(CMAKE_C_COMPILER "C:/Program Files/mingw-w64/x86_64-8.1.0-posix-seh-rt_v6-rev0/mingw64/bin/gcc.exe")
#set(CMAKE_CXX_COMPILER "C:/Program Files/mingw-w64/x86_64-8.1.0-posix-seh-rt_v6-rev0/mingw64/bin/g++.exe")
cmake_minimum_required(VERSION 3.13)
project(PathtracerReloaded)
add_compile_options(-m64)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_INCLUDE_PATH src/myGL)

set(BUILD_SHARED_LIBS OFF)
#set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-static")



set(GLUT_INCLUDE_DIR D:/myGL)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lz -lpng16 -pthread -static-libstdc++ -static-libgcc -Ofast -ffunction-sections -fomit-frame-pointer -march=native -mavx2")# -fno-inline")# -O3 -march=native")# -O1  -mavx2 -floop-parallelize-all
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} `pkg-config --cflags --libs gtk+-2.0")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx512f")

include (FindPkgConfig)
if (NOT PKG_CONFIG_FOUND)
    message (FATAL_ERROR "pkg-config not found")
endif ()
# check for libpng
pkg_check_modules (LIBPNG libpng16 REQUIRED)
if (NOT LIBPNG_FOUND)
    message(FATAL_ERROR "You don't seem to have libpng16 development libraries installed")
else ()
    include_directories (${LIBPNG_INCLUDE_DIRS})
    link_directories (${LIBPNG_LIBRARY_DIRS})
    link_libraries (${LIBPNG_LIBRARIES})
endif ()

pkg_check_modules (GTK3 REQUIRED gtk+-3.0)
pkg_check_modules (GLIB REQUIRED)

include_directories (${GTK3_INCLUDE_DIRS})


ADD_DEFINITIONS(${GTK3_CFLAGS_OTHER})

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
link_directories(${OPENGL_gl_LIBRARY} ${GTK3_LIBRARY_DIRS})

add_executable(PathtracerReloaded src/main.cpp src/myGL/gl.h src/myGL/glext.h src/myGL/freeglut.h src/include.h src/define.h src/base_utility.h src/Vec3f.h
                                  src/GXX.h src/OrenNayar.h src/Material.h src/Allocator.h src/Ray.h src/AABB.h src/Hit.h src/Primitive.h src/Triangle.h src/utility.h
								  src/Accelerator.h src/BVH.h src/Light.h src/Camera.h src/Scene.h src/Import.h src/OpenGL_utility.h src/Integrator.h src/BasicIntegrator.h src/Render.h )


target_link_libraries(PathtracerReloaded ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GTK3_LIBRARIES} ${LIBPNG_LIBRARIES} pthread)

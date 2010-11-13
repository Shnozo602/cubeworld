#########################################################################
### Project source build script                                       ###
###  - This portion of the build will compile all testable code into  ###
###    a static library, and then link the entire project binary      ###
###    together along with any required libraries and settings        ###
#########################################################################
# Core source code files that comprise cube world's game engine. This is
# all non-client presentation code
SET(engine_srcs
        gameworld.cpp
        typesio.cpp
        cubeworld.cpp)
		
# Client code. All testable code that makes the client goes in here.
SET(client_srcs
		camera.cpp
		gameapp.cpp)

# Client code that is not under test
SET(client_notest_srcs
        rogue.cpp
        cubeworldbuilder.cpp
        renderers/opengl.cpp)

#========================================================================
# Internal engine libraries
#========================================================================
set(ENGINE_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/libs/appcore/include"
                        "${CMAKE_SOURCE_DIR}/libs/math/include")
set(ENGINE_LIBS shared_corelib shared_tga shared_math)

#========================================================================
# External required libraries
#========================================================================
find_package(GLUT REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

# Make sure they were actually pulled in (it seems cmake ignores REQUIRED)
if(NOT OPENGL_FOUND)
	message(FATAL_ERROR "OpenGL development libs not found")
endif()

if(NOT GLUT_FOUND)
    message(FATAL_ERROR "Freeglut library not found")
endif()

if(NOT GLEW_FOUND)
    message(FATAL_ERROR "GLEW library not found")
endif()

# Pull in required third party libraries (OpenGL, etc)
set(APP_LIBS ${EXTRA_LIBS} 
             ${GLUT_LIBRARIES}
             ${OpenGL_LIBRARIES}
             ${GLEW_LIBRARY}
             ${GLU_LIBRARY}
             lodepng)
			 
set(APP_LIB_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/thirdparty/lodepng/include)

#========================================================================
# Application compiling and linking
#========================================================================
# Set up include directories
include_directories( ${CMAKE_SOURCE_DIR}/include
                     ${APP_LIB_INCLUDE_DIRS}
                     ${ENGINE_INCLUDE_DIRS} )

add_definitions("-DFREEGLUT_STATIC -DGLEW_STATIC")
					 
#
# Create a static library for the cubeworld game engine code. This will 
# seperate the client from the actual game logic.
#
add_library(cubeworld_engine STATIC ${engine_srcs})

set_target_properties(cubeworld_engine PROPERTIES COMPILE_FLAGS "${cxx_flags}")

#
# Create the executable that will run the game client
#
add_executable(${APP_BIN_NAME} ${client_srcs} ${client_notest_srcs})

set_target_properties(${APP_BIN_NAME} PROPERTIES COMPILE_FLAGS "${cxx_flags}")

# Link any needed libraries
target_link_libraries(${APP_BIN_NAME}
                      cubeworld_engine ${ENGINE_LIBS} ${APP_LIBS})

#==========================================================================
# Run the target
#==========================================================================
if(NOT MSVC)
    add_custom_target(run ${APP_BIN_NAME} DEPENDS ${APP_BIN_NAME}
                      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                      COMMENT "Runs the cube world client")

    add_custom_target(debug 
                     gdb `pwd`/${APP_BIN_NAME} -d ${PROJECT_SOURCE_DIR}/src -cd
                     ${PROJECT_SOURCE_DIR}
                     DEPENDS ${APP_BIN_NAME}
                     COMMENT "Run cubeworld client with GDB support")
endif()
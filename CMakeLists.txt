cmake_minimum_required(VERSION 3.16.0)

message("Build type: ${CMAKE_BUILD_TYPE}")

# this might not be needed anymore
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Define our project name
set(PROJECT_NAME vellocet3d)
project(${PROJECT_NAME})

option(DISPLAY_DEBUG_OUTPUT "Turn on during development. Will give useful messages as to where certain problems exist. Turn off for production" ON)
if(DISPLAY_DEBUG_OUTPUT)
	add_definitions(-DDEBUG_LOG)
endif()


if(MSVC)
	
	# this might not be needed anymore
	add_compile_options(
        $<$<CONFIG:>:/MT> #---------|
        $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
        $<$<CONFIG:Release>:/MT> #--|
    )
	
	get_filename_component(_vs_bin_path "${CMAKE_LINKER}" DIRECTORY)
	set(libexe "${_vs_bin_path}/lib.exe")
	
	set(NVIDIA_API_LIB_PATH "" CACHE STRING "")
	set(NVIDIA_API_LIB_PATH ${NVIDIA_API_LIB_PATH} CACHE STRING "Path to nvapi64.lib, including the filename")
	
	set(NVIDIA_API_INCLUDE_PATH "" CACHE STRING "")
	set(NVIDIA_API_INCLUDE_PATH ${NVIDIA_API_INCLUDE_PATH} CACHE STRING "Path to nvidia api include files")
	
	add_definitions(-DWINDOWS_BUILD)
	
endif()

# Make sure binary directory is not the same as source directory
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()


# This Project Depends on External Project(s) 
include(ExternalProject)
include(FetchContent)

# SETUP GLFW
set(libGLFW glfw)
ExternalProject_Add(${libGLFW}
PREFIX			${CMAKE_CURRENT_BINARY_DIR}/_deps/${libGLFW}
GIT_REPOSITORY  https://github.com/glfw/glfw.git
GIT_TAG         3.3.4
GIT_SHALLOW     ON
UPDATE_COMMAND 	""
CMAKE_ARGS      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/_deps/installs/${libGLFW}
				-DGLFW_BUILD_DOCS:BOOL=OFF
				-DGLFW_BUILD_EXAMPLES:BOOL=OFF
				-DGLFW_BUILD_TESTS:BOOL=OFF
				-DUSE_MSVC_RUNTIME_LIBRARY_DLL:BOOL=OFF
				-DGLFW_USE_HYBRID_HPG:BOOL=ON
)

set(GLFW_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/installs/${libGLFW})
add_library(GLFW_LIBRARY STATIC IMPORTED)
set_target_properties(GLFW_LIBRARY PROPERTIES IMPORTED_LOCATION ${GLFW_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glfw3${CMAKE_STATIC_LIBRARY_SUFFIX})



# SETUP GLAD
set(libGLAD glad)
ExternalProject_Add(${libGLAD}
PREFIX			${CMAKE_CURRENT_BINARY_DIR}/_deps/${libGLAD}
GIT_REPOSITORY  https://github.com/Dav1dde/glad.git
GIT_TAG         v0.1.36
GIT_SHALLOW     ON
UPDATE_COMMAND 	""
CMAKE_ARGS      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/_deps/installs/${libGLAD}
				-DGLAD_INSTALL:BOOL=ON
				-DGLAD_PROFILE:STRING="core"
				-DGLAD_ALL_EXTENSIONS:BOOL=ON
				-DUSE_MSVC_RUNTIME_LIBRARY_DLL:BOOL=OFF
)

set(GLAD_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/installs/${libGLAD})
add_library(GLAD_LIBRARY STATIC IMPORTED)
set_target_properties(GLAD_LIBRARY PROPERTIES IMPORTED_LOCATION ${GLAD_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glad${CMAKE_STATIC_LIBRARY_SUFFIX})


# SETUP GLM, need to add a GIT_TAG when they do the next latest release as we have to use master branch for the updated cmake file, but we need to insure stability which can only happen with a concrete release, since code in master can change at any point
set(libGLM glm)
ExternalProject_Add(${libGLM}
PREFIX			${CMAKE_CURRENT_BINARY_DIR}/_deps/${libGLM}
GIT_REPOSITORY  https://github.com/g-truc/glm.git
GIT_TAG         origin/master
GIT_SHALLOW     ON
UPDATE_COMMAND 	""
CMAKE_ARGS      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/_deps/installs/${libGLM}
				-DBUILD_SHARED_LIBS:BOOL=OFF
				-DBUILD_STATIC_LIBS:BOOL=OFF
				-DGLM_TEST_ENABLE:BOOL=OFF
)

set(GLM_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/installs/${libGLM})


# SETUP ASSIMP
set(libAssimp assimp)
ExternalProject_Add(${libAssimp}
PREFIX			${CMAKE_CURRENT_BINARY_DIR}/_deps/${libAssimp}
GIT_REPOSITORY  https://github.com/assimp/assimp.git
GIT_TAG         v5.0.1
GIT_SHALLOW     ON
UPDATE_COMMAND 	""
CMAKE_ARGS      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/_deps/installs/${libAssimp}
				-DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT:BOOL=OFF
				-DASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT:BOOL=OFF
				-DASSIMP_BUILD_ASSIMP_TOOLS:BOOL=OFF
				-DASSIMP_BUILD_TESTS:BOOL=OFF
				-DASSIMP_BUILD_FBX_IMPORTER:BOOL=ON
				-DASSIMP_BUILD_OBJ_IMPORTER:BOOL=ON
				-DASSIMP_BUILD_OBJ_EXPORTER:BOOL=ON
				-DASSIMP_BUILD_ZLIB:BOOL=ON
				-DASSIMP_LIBRARY_SUFFIX:STRING=
				-DLIBRARY_SUFFIX:STRING=
				-DCMAKE_DEBUG_POSTFIX:STRING=
				-DASSIMP_INJECT_DEBUG_POSTFIX:BOOL=OFF
				-DBUILD_SHARED_LIBS:BOOL=OFF
				-DCMAKE_POLICY_DEFAULT_CMP0091:STRING=NEW
				-DCMAKE_MSVC_RUNTIME_LIBRARY:STRING=MultiThreaded$<$<CONFIG:Debug>:Debug>
)

set(ASSIMP_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/installs/${libAssimp})
add_library(ASSIMP_LIBRARY STATIC IMPORTED)
set_target_properties(ASSIMP_LIBRARY PROPERTIES IMPORTED_LOCATION ${ASSIMP_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}assimp${CMAKE_STATIC_LIBRARY_SUFFIX})

add_library(IRRXML_LIBRARY STATIC IMPORTED)
set_target_properties(IRRXML_LIBRARY PROPERTIES IMPORTED_LOCATION ${ASSIMP_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}IrrXML${CMAKE_STATIC_LIBRARY_SUFFIX})

add_library(ZLIB_LIBRARY STATIC IMPORTED)
set_target_properties(
ZLIB_LIBRARY 
PROPERTIES 
	IMPORTED_LOCATION_RELEASE ${ASSIMP_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}zlibstatic${CMAKE_STATIC_LIBRARY_SUFFIX}
	IMPORTED_LOCATION_MINSIZEREL ${ASSIMP_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}zlibstatic${CMAKE_STATIC_LIBRARY_SUFFIX}
	IMPORTED_LOCATION_RELWITHDEBINFO ${ASSIMP_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}zlibstatic${CMAKE_STATIC_LIBRARY_SUFFIX}
	IMPORTED_LOCATION_DEBUG ${ASSIMP_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}zlibstaticd${CMAKE_STATIC_LIBRARY_SUFFIX}
)


# SETUP BULLET
set(libBullet bullet)
ExternalProject_Add(${libBullet}
PREFIX			${CMAKE_CURRENT_BINARY_DIR}/_deps/${libBullet}
GIT_REPOSITORY  https://github.com/bulletphysics/bullet3.git
GIT_TAG         3.17
GIT_SHALLOW     ON
UPDATE_COMMAND 	""
CMAKE_ARGS      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/_deps/installs/${libBullet}
				-DINSTALL_LIBS:BOOL=ON
				-DBUILD_BULLET2_DEMOS:BOOL=OFF
				-DBUILD_BULLET3:BOOL=OFF
				-DBUILD_CLSOCKET:BOOL=OFF
				-DBUILD_CPU_DEMOS:BOOL=OFF
				-DBUILD_EGL:BOOL=OFF
				-DBUILD_ENET:BOOL=OFF
				-DBUILD_EXTRAS:BOOL=OFF
				-DBUILD_OPENGL3_DEMOS:BOOL=OFF
				-DBUILD_UNIT_TESTS:BOOL=OFF
				-DENABLE_VHACD:BOOL=OFF
				-DINSTALL_CMAKE_FILES:BOOL=OFF
				-DINSTALL_EXTRA_LIBS:BOOL=OFF
				-DUSE_GLUT:BOOL=OFF
				-DUSE_GRAPHICAL_BENCHMARK:BOOL=OFF
)


set(BULLET_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/installs/${libBullet})

add_library(BULLET_COLLISION_LIBRARY STATIC IMPORTED)
add_library(BULLET_DYNAMICS_LIBRARY STATIC IMPORTED)
add_library(BULLET_INVERSE_DYNAMICS_LIBRARY STATIC IMPORTED)
add_library(BULLET_SOFT_BODY_LIBRARY STATIC IMPORTED)
add_library(LINEAR_MATH_LIBRARY STATIC IMPORTED)

if(MSVC)
	set_target_properties(
	BULLET_COLLISION_LIBRARY 
	PROPERTIES 
		IMPORTED_LOCATION_RELEASE ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}BulletCollision${CMAKE_STATIC_LIBRARY_SUFFIX}
		IMPORTED_LOCATION_MINSIZEREL ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}BulletCollision_MinsizeRel${CMAKE_STATIC_LIBRARY_SUFFIX}
		IMPORTED_LOCATION_RELWITHDEBINFO ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}BulletCollision_RelWithDebugInfo${CMAKE_STATIC_LIBRARY_SUFFIX}
		IMPORTED_LOCATION_DEBUG ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}BulletCollision_Debug${CMAKE_STATIC_LIBRARY_SUFFIX}
	)
	
	set_target_properties(
	BULLET_DYNAMICS_LIBRARY 
	PROPERTIES 
		IMPORTED_LOCATION_RELEASE ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}BulletDynamics${CMAKE_STATIC_LIBRARY_SUFFIX}
		IMPORTED_LOCATION_MINSIZEREL ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}BulletDynamics_MinsizeRel${CMAKE_STATIC_LIBRARY_SUFFIX}
		IMPORTED_LOCATION_RELWITHDEBINFO ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}BulletDynamics_RelWithDebugInfo${CMAKE_STATIC_LIBRARY_SUFFIX}
		IMPORTED_LOCATION_DEBUG ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}BulletDynamics_Debug${CMAKE_STATIC_LIBRARY_SUFFIX}
	)
	
	set_target_properties(
	BULLET_INVERSE_DYNAMICS_LIBRARY 
	PROPERTIES 
		IMPORTED_LOCATION_RELEASE ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}BulletInverseDynamics${CMAKE_STATIC_LIBRARY_SUFFIX}
		IMPORTED_LOCATION_MINSIZEREL ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}BulletInverseDynamics_MinsizeRel${CMAKE_STATIC_LIBRARY_SUFFIX}
		IMPORTED_LOCATION_RELWITHDEBINFO ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}BulletInverseDynamics_RelWithDebugInfo${CMAKE_STATIC_LIBRARY_SUFFIX}
		IMPORTED_LOCATION_DEBUG ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}BulletInverseDynamics_Debug${CMAKE_STATIC_LIBRARY_SUFFIX}
	)
	
	set_target_properties(
	BULLET_SOFT_BODY_LIBRARY 
	PROPERTIES 
		IMPORTED_LOCATION_RELEASE ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}BulletSoftBody${CMAKE_STATIC_LIBRARY_SUFFIX}
		IMPORTED_LOCATION_MINSIZEREL ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}BulletSoftBody_MinsizeRel${CMAKE_STATIC_LIBRARY_SUFFIX}
		IMPORTED_LOCATION_RELWITHDEBINFO ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}BulletSoftBody_RelWithDebugInfo${CMAKE_STATIC_LIBRARY_SUFFIX}
		IMPORTED_LOCATION_DEBUG ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}BulletSoftBody_Debug${CMAKE_STATIC_LIBRARY_SUFFIX}
	)
	
	set_target_properties(
	LINEAR_MATH_LIBRARY 
	PROPERTIES 
		IMPORTED_LOCATION_RELEASE ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}LinearMath${CMAKE_STATIC_LIBRARY_SUFFIX}
		IMPORTED_LOCATION_MINSIZEREL ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}LinearMath_MinsizeRel${CMAKE_STATIC_LIBRARY_SUFFIX}
		IMPORTED_LOCATION_RELWITHDEBINFO ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}LinearMath_RelWithDebugInfo${CMAKE_STATIC_LIBRARY_SUFFIX}
		IMPORTED_LOCATION_DEBUG ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}LinearMath_Debug${CMAKE_STATIC_LIBRARY_SUFFIX}
	)
	
	
else()
	set_target_properties(BULLET_COLLISION_LIBRARY PROPERTIES IMPORTED_LOCATION ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}BulletCollision${CMAKE_STATIC_LIBRARY_SUFFIX})
	set_target_properties(BULLET_DYNAMICS_LIBRARY PROPERTIES IMPORTED_LOCATION ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}BulletDynamics${CMAKE_STATIC_LIBRARY_SUFFIX})
	set_target_properties(BULLET_INVERSE_DYNAMICS_LIBRARY PROPERTIES IMPORTED_LOCATION ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}BulletInverseDynamics${CMAKE_STATIC_LIBRARY_SUFFIX})
	set_target_properties(BULLET_SOFT_BODY_LIBRARY PROPERTIES IMPORTED_LOCATION ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}BulletSoftBody${CMAKE_STATIC_LIBRARY_SUFFIX})
	set_target_properties(LINEAR_MATH_LIBRARY PROPERTIES IMPORTED_LOCATION ${BULLET_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}LinearMath${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()



# SETUP JSON
set(libJson json)
FetchContent_Declare(${libJson}
	GIT_REPOSITORY https://github.com/nlohmann/json.git
	GIT_TAG v3.10.5
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
)

FetchContent_GetProperties(${libJson})
if(NOT ${libJson})
  FetchContent_Populate(${libJson})
endif()

add_library(${libJson} INTERFACE)

file(
COPY
	${${libJson}_SOURCE_DIR}/single_include/nlohmann/json.hpp
DESTINATION
	${CMAKE_CURRENT_BINARY_DIR}/_deps/installs/${libJson}/include/nlohmann
)

set(JSON_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/installs/${libJson})


# SETUP STB_IMAGE, doesn't look like they do releases, but using master for this shouldn't be an issue
set(libStbImage stb_image)
FetchContent_Declare(${libStbImage}
	GIT_REPOSITORY https://github.com/nothings/stb.git
	GIT_TAG origin/master
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
)

FetchContent_GetProperties(${libStbImage})
if(NOT ${libStbImage})
  FetchContent_Populate(${libStbImage})
endif()

add_library(${libStbImage} INTERFACE)

file(
COPY
	${${libStbImage}_SOURCE_DIR}/stb_image.h
DESTINATION
	${CMAKE_CURRENT_BINARY_DIR}/_deps/installs/${libStbImage}/include/${libStbImage}
)

set(STBIMAGE_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/installs/${libStbImage})



# SETUP IMGUI
set(libImGui imgui)
FetchContent_Declare(${libImGui}
	GIT_REPOSITORY https://github.com/ocornut/imgui.git
	GIT_TAG v1.84.1
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
)

FetchContent_GetProperties(${libImGui})
if(NOT ${libImGui})
  FetchContent_Populate(${libImGui})
endif()

file(
COPY
	${${libImGui}_SOURCE_DIR}/imgui.cpp
	${${libImGui}_SOURCE_DIR}/imgui_draw.cpp
	${${libImGui}_SOURCE_DIR}/imgui_tables.cpp
	${${libImGui}_SOURCE_DIR}/imgui_widgets.cpp
	${${libImGui}_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
	${${libImGui}_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
DESTINATION
	${CMAKE_CURRENT_BINARY_DIR}/_deps/installs/${libImGui}/src
)

file(
COPY
	${${libImGui}_SOURCE_DIR}/imconfig.h
	${${libImGui}_SOURCE_DIR}/imgui.h
	${${libImGui}_SOURCE_DIR}/imgui_internal.h
	${${libImGui}_SOURCE_DIR}/imstb_rectpack.h
	${${libImGui}_SOURCE_DIR}/imstb_textedit.h
	${${libImGui}_SOURCE_DIR}/imstb_truetype.h
	${${libImGui}_SOURCE_DIR}/backends/imgui_impl_glfw.h
	${${libImGui}_SOURCE_DIR}/backends/imgui_impl_opengl3.h
	${${libImGui}_SOURCE_DIR}/backends/imgui_impl_opengl3_loader.h
DESTINATION
	${CMAKE_CURRENT_BINARY_DIR}/_deps/installs/${libImGui}/include/${libImGui}
)

set(IMGUI_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/installs/${libImGui})

file(GLOB_RECURSE imgui_headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/_deps/installs/${libImGui}/include/${libImGui}/*.h")
file(GLOB_RECURSE imgui_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/_deps/installs/${libImGui}/src/*.cpp")
add_library(IMGUI_LIBRARY STATIC ${imgui_headers} ${imgui_sources})
add_dependencies(IMGUI_LIBRARY ${libGLFW})

set_target_properties(IMGUI_LIBRARY PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${IMGUI_INSTALL_DIR}/lib
)

target_include_directories(IMGUI_LIBRARY
	PUBLIC ${IMGUI_INSTALL_DIR}/include/${libImGui}
	PUBLIC ${GLFW_INSTALL_DIR}/include
)



# SETUP CDT
set(libCDT cdt)
ExternalProject_Add(${libCDT}
PREFIX			${CMAKE_CURRENT_BINARY_DIR}/_deps/${libCDT}
GIT_REPOSITORY  https://github.com/artem-ogre/CDT.git
SOURCE_SUBDIR 	"CDT"
GIT_TAG         1.1.2
GIT_SHALLOW     ON
UPDATE_COMMAND 	""
CMAKE_ARGS      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/_deps/installs/${libCDT}
				-DCDT_USE_64_BIT_INDEX_TYPE:BOOL=ON
				-DCDT_USE_AS_COMPILED_LIBRARY:BOOL=ON
)

set(CDT_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/installs/${libCDT})
add_library(CDT_LIBRARY STATIC IMPORTED)
set_target_properties(CDT_LIBRARY PROPERTIES IMPORTED_LOCATION ${CDT_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}cdt${CMAKE_STATIC_LIBRARY_SUFFIX})



# SETUP NVAPI IF ON WINDOWS AND PATH GIVEN
if(MSVC)
	if(NVIDIA_API_LIB_PATH STREQUAL "" OR NVIDIA_API_INCLUDE_PATH STREQUAL "")
		#message("Skipping Nvidia api inclusion...you're going to see random intermittent stuttering unless you turn off Threaded Optimization globally")
	else()
		add_library(NVAPI_LIBRARY STATIC IMPORTED)
		set_target_properties(NVAPI_LIBRARY PROPERTIES IMPORTED_LOCATION ${NVIDIA_API_LIB_PATH})
	endif()
endif()


# SETUP OUR LIBRARY
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(${PROJECT_NAME} STATIC ${headers} ${sources})
add_dependencies(${PROJECT_NAME} ${libGLFW} ${libGLAD} ${libGLM} ${libAssimp} ${libBullet} ${libJson} ${libStbImage} ${libCDT} IMGUI_LIBRARY)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

if(MSVC AND NOT NVIDIA_API_LIB_PATH STREQUAL "" AND NOT NVIDIA_API_INCLUDE_PATH STREQUAL "")
	add_dependencies(${PROJECT_NAME} NVAPI_LIBRARY)
endif()


# Add all include file paths
target_include_directories(${PROJECT_NAME}
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
	PUBLIC ${GLFW_INSTALL_DIR}/include
	PUBLIC ${CDT_INSTALL_DIR}/include
	PUBLIC ${GLAD_INSTALL_DIR}/include
	PUBLIC ${GLM_INSTALL_DIR}/include
	PUBLIC ${ASSIMP_INSTALL_DIR}/include
	PUBLIC ${BULLET_INSTALL_DIR}/include/bullet
	PUBLIC ${JSON_INSTALL_DIR}/include 
	PUBLIC ${STBIMAGE_INSTALL_DIR}/include
	PUBLIC ${IMGUI_INSTALL_DIR}/include
)

if(MSVC AND NOT NVIDIA_API_LIB_PATH STREQUAL "" AND NOT NVIDIA_API_INCLUDE_PATH STREQUAL "")
	target_include_directories(${PROJECT_NAME} PUBLIC ${NVIDIA_API_INCLUDE_PATH})
endif()

# This SHOULD link the required libraries along with the project library if 
# down the road an executable is built and only links this project library
# (or at least that's what I read somewhere)
target_link_libraries(${PROJECT_NAME} PUBLIC
	GLFW_LIBRARY 
	GLAD_LIBRARY
	ASSIMP_LIBRARY
	IRRXML_LIBRARY
	ZLIB_LIBRARY
	BULLET_COLLISION_LIBRARY
	BULLET_DYNAMICS_LIBRARY
	BULLET_INVERSE_DYNAMICS_LIBRARY
	BULLET_SOFT_BODY_LIBRARY
	LINEAR_MATH_LIBRARY
	IMGUI_LIBRARY
	CDT_LIBRARY
)

if(MSVC AND NOT NVIDIA_API_LIB_PATH STREQUAL "" AND NOT NVIDIA_API_INCLUDE_PATH STREQUAL "")
	target_link_libraries(${PROJECT_NAME} PUBLIC NVAPI_LIBRARY)
endif()


# Create an EXPORT project that joins all libraries and exports their include files to
# buildpath/export so there is a singular location where someone can just build this project,
# go to the export directory, and grab the static library file and a singular directory containing
# the includes that they might need so they don't have to dig through a bunch of different directories
# and build seperate projects...etc. For the non-cmake inclinded individuals (ie myself before starting
# this process) who just want to get going as fast as possible without using cmake for their project.

# Also, this might make creating an install target easier? Although, if someone wants to install but use
# packages they have installed on their system (bullet/assimp/glfw/glad/etc) this would force them to use
# the dependency versions we downloaded and built...so maybe down the road set this up to use find_package()?
# IDK, because if we require a specific version of the library, but user has a different version and they 
# try to build with that then they will encounter issues and blame this library. I would think it would be
# better to encapsulate the specific dependency versions and package together as a standalone static library...
# but maybe I'm wrong...?
set(LIBNAME "${CMAKE_CURRENT_BINARY_DIR}/export/${CMAKE_STATIC_LIBRARY_PREFIX}${PROJECT_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")

if(MSVC)
	if(NOT NVIDIA_API_LIB_PATH STREQUAL "" AND NOT NVIDIA_API_INCLUDE_PATH STREQUAL "")
		add_custom_command(
			OUTPUT ${LIBNAME}
			COMMAND ${libexe} /OUT:${LIBNAME} $<TARGET_FILE:${PROJECT_NAME}> $<TARGET_FILE:GLFW_LIBRARY> $<TARGET_FILE:CDT_LIBRARY> $<TARGET_FILE:GLAD_LIBRARY> $<TARGET_FILE:ASSIMP_LIBRARY> $<TARGET_FILE:IRRXML_LIBRARY> $<TARGET_FILE:ZLIB_LIBRARY> $<TARGET_FILE:BULLET_COLLISION_LIBRARY> $<TARGET_FILE:BULLET_DYNAMICS_LIBRARY> $<TARGET_FILE:BULLET_INVERSE_DYNAMICS_LIBRARY> $<TARGET_FILE:BULLET_SOFT_BODY_LIBRARY> $<TARGET_FILE:LINEAR_MATH_LIBRARY> $<TARGET_FILE:IMGUI_LIBRARY> $<TARGET_FILE:NVAPI_LIBRARY>
			COMMAND xcopy /E /I /Y \"${CMAKE_CURRENT_SOURCE_DIR}/inc\" \"${CMAKE_CURRENT_BINARY_DIR}/export/include\"
			COMMAND xcopy /E /I /Y \"${GLM_INSTALL_DIR}/include\" \"${CMAKE_CURRENT_BINARY_DIR}/export/include\"
			COMMAND xcopy /E /I /Y \"${CDT_INSTALL_DIR}/include\" \"${CMAKE_CURRENT_BINARY_DIR}/export/include\"
			COMMAND xcopy /E /I /Y \"${IMGUI_INSTALL_DIR}/include\" \"${CMAKE_CURRENT_BINARY_DIR}/export/include\"
			COMMAND xcopy /E /I /Y \"${NVIDIA_API_INCLUDE_PATH}\" \"${CMAKE_CURRENT_BINARY_DIR}/export/include\"
			COMMAND xcopy /E /I /Y \"${BULLET_INSTALL_DIR}/include\" \"${CMAKE_CURRENT_BINARY_DIR}/export/include\"
			COMMAND xcopy /E /I /Y \"${JSON_INSTALL_DIR}/include\" \"${CMAKE_CURRENT_BINARY_DIR}/export/include\"
			DEPENDS GLFW_LIBRARY CDT_LIBRARY GLAD_LIBRARY ASSIMP_LIBRARY BULLET_COLLISION_LIBRARY BULLET_DYNAMICS_LIBRARY BULLET_INVERSE_DYNAMICS_LIBRARY BULLET_SOFT_BODY_LIBRARY LINEAR_MATH_LIBRARY IMGUI_LIBRARY ${PROJECT_NAME} NVAPI_LIBRARY
			COMMENT "Exporting ${PROJECT_NAME}..."
		)
	else()
		add_custom_command(
			OUTPUT ${LIBNAME}
			COMMAND ${libexe} /OUT:${LIBNAME} $<TARGET_FILE:${PROJECT_NAME}> $<TARGET_FILE:GLFW_LIBRARY> $<TARGET_FILE:CDT_LIBRARY> $<TARGET_FILE:GLAD_LIBRARY> $<TARGET_FILE:ASSIMP_LIBRARY> $<TARGET_FILE:IRRXML_LIBRARY> $<TARGET_FILE:ZLIB_LIBRARY> $<TARGET_FILE:BULLET_COLLISION_LIBRARY> $<TARGET_FILE:BULLET_DYNAMICS_LIBRARY> $<TARGET_FILE:BULLET_INVERSE_DYNAMICS_LIBRARY> $<TARGET_FILE:BULLET_SOFT_BODY_LIBRARY> $<TARGET_FILE:LINEAR_MATH_LIBRARY> $<TARGET_FILE:IMGUI_LIBRARY>
			COMMAND xcopy /E /I /Y \"${CMAKE_CURRENT_SOURCE_DIR}/inc\" \"${CMAKE_CURRENT_BINARY_DIR}/export/include\"
			COMMAND xcopy /E /I /Y \"${GLM_INSTALL_DIR}/include\" \"${CMAKE_CURRENT_BINARY_DIR}/export/include\"
			COMMAND xcopy /E /I /Y \"${CDT_INSTALL_DIR}/include\" \"${CMAKE_CURRENT_BINARY_DIR}/export/include\"
			COMMAND xcopy /E /I /Y \"${IMGUI_INSTALL_DIR}/include\" \"${CMAKE_CURRENT_BINARY_DIR}/export/include\"
			COMMAND xcopy /E /I /Y \"${BULLET_INSTALL_DIR}/include\" \"${CMAKE_CURRENT_BINARY_DIR}/export/include\"
			COMMAND xcopy /E /I /Y \"${JSON_INSTALL_DIR}/include\" \"${CMAKE_CURRENT_BINARY_DIR}/export/include\"
			DEPENDS GLFW_LIBRARY GLAD_LIBRARY ASSIMP_LIBRARY BULLET_COLLISION_LIBRARY BULLET_DYNAMICS_LIBRARY BULLET_INVERSE_DYNAMICS_LIBRARY BULLET_SOFT_BODY_LIBRARY LINEAR_MATH_LIBRARY IMGUI_LIBRARY ${PROJECT_NAME}
			COMMENT "Exporting ${PROJECT_NAME}..."
		)
	endif()
elseif(UNIX)
	add_custom_command(
		OUTPUT ${LIBNAME}
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/export_lib.sh ${LIBNAME} $<TARGET_FILE:${PROJECT_NAME}> $<TARGET_FILE:GLFW_LIBRARY> $<TARGET_FILE:CDT_LIBRARY> $<TARGET_FILE:GLAD_LIBRARY> $<TARGET_FILE:ASSIMP_LIBRARY> $<TARGET_FILE:IRRXML_LIBRARY> $<TARGET_FILE:ZLIB_LIBRARY> $<TARGET_FILE:BULLET_COLLISION_LIBRARY> $<TARGET_FILE:BULLET_DYNAMICS_LIBRARY> $<TARGET_FILE:BULLET_INVERSE_DYNAMICS_LIBRARY> $<TARGET_FILE:BULLET_SOFT_BODY_LIBRARY> $<TARGET_FILE:LINEAR_MATH_LIBRARY> $<TARGET_FILE:IMGUI_LIBRARY>
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/export_include.sh ${CMAKE_CURRENT_BINARY_DIR}/export/include ${GLM_INSTALL_DIR}/include ${CDT_INSTALL_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/inc ${BULLET_INSTALL_DIR}/include ${IMGUI_INSTALL_DIR}/include ${BULLET_INSTALL_DIR}/include ${JSON_INSTALL_DIR}/include
		DEPENDS GLFW_LIBRARY CDT_LIBRARY GLAD_LIBRARY ASSIMP_LIBRARY BULLET_COLLISION_LIBRARY BULLET_DYNAMICS_LIBRARY BULLET_INVERSE_DYNAMICS_LIBRARY BULLET_SOFT_BODY_LIBRARY LINEAR_MATH_LIBRARY IMGUI_LIBRARY ${PROJECT_NAME}
		COMMENT "Exporting ${PROJECT_NAME}..."
	)
endif()



add_custom_target(EXPORT_LIB
    DEPENDS ${LIBNAME}
)

# add_library(combinedLib STATIC IMPORTED)
# set_property(TARGET combinedLib PROPERTY IMPORTED_LOCATION ${LIBNAME})
# add_dependencies(combinedLib EXPORT)
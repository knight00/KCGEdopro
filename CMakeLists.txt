cmake_minimum_required(VERSION 3.12.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(EDOPro)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

option(EDOPRO_MSVC_STATIC "Static CRT on Visual Studio" ON)
option(EDOPRO_PICS_URL "Default card art URL template" $ENV{PICS_URL})
option(EDOPRO_FIELDS_URL "Default field background URL template" $ENV{FIELDS_URL})
option(EDOPRO_DISCORD_APP_ID "Discord App ID" $ENV{DISCORD_APP_ID})

if(NOT DEFINED WIN32)
	find_package(Freetype REQUIRED)
	find_package(Irrlicht REQUIRED)
endif()

find_package(Libevent CONFIG REQUIRED)
find_package(sqlite3 REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(libgit2 REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_library(DISCORD_RPC_PAYLOAD_LIBRARY NAMES discord-rpc)

find_package(Threads REQUIRED)
if(APPLE)
	find_library(COCOA_FRAMEWORK Cocoa)
	find_library(IOKIT_FRAMEWORK IOKit)
	find_library(GL_FRAMEWORK OpenGL)
elseif(UNIX)
	find_package(OpenGL REQUIRED)
	find_package(X11 REQUIRED)
endif()

# Doesn't get set on Linux
if(NOT DEFINED SQLite3_LIBRARY)
	set(SQLite3_LIBRARY sqlite3)
endif()

if(NOT DEFINED LIBGIT2_LIBRARIES)
	set(LIBGIT2_LIBRARIES git2)
endif()

add_executable(EDOPro)
add_executable(EDOProStatic)
set_property (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "EDOProStatic")

target_compile_definitions(EDOPro PUBLIC YGOPRO_BUILD_DLL)

target_include_directories(EDOPro PUBLIC
	${FREETYPE_INCLUDE_DIR_freetype2}
	${SQLite3_INCLUDE_DIR}
	${IRRLICHT_INCLUDE_DIR}
	${LIBGIT2_INCLUDE_DIRS}
	ocgcore)

target_link_libraries(EDOPro PRIVATE
	clzma
	event_static event_core_static event_extra_static event_pthreads_static
	${FREETYPE_LIBRARY}
	${SQLite3_LIBRARY}
	${IRRLICHT_LIBRARY}
	CURL::libcurl
	${LIBGIT2_LIBRARIES}
	${DISCORD_RPC_PAYLOAD_LIBRARY}
	fmt::fmt
	nlohmann_json::nlohmann_json
	Threads::Threads)

if(APPLE)
	target_link_libraries(EDOPro PRIVATE
		${COCOA_FRAMEWORK}
		${IOKIT_FRAMEWORK}
		${GL_FRAMEWORK})
elseif(UNIX)
	target_link_libraries(EDOPro PRIVATE
		OpenGL::GL
		${X11_LIBRARIES})
endif()

add_subdirectory(ocgcore)
add_subdirectory(gframe)
if(WIN32)
	add_subdirectory(freetype)
	add_subdirectory(irrlicht)
endif()

set(CLANG_OR_GNU $<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>)
set(EDOPRO_COMPILE_OPTIONS
	# Clang and GNU compilers
	$<${CLANG_OR_GNU}:-Wall -Wextra -pedantic>
	$<$<AND:$<CONFIG:Debug>,${CLANG_OR_GNU}>:-ggdb3 -Og -D_DEBUG>
	$<$<AND:$<CONFIG:Release>,${CLANG_OR_GNU}>:-O3>
	$<$<AND:$<CONFIG:RelWithDebInfo>,${CLANG_OR_GNU}>:-ggdb3 -O3 -D_DEBUG>
	# Microsoft compiler
	$<$<CXX_COMPILER_ID:MSVC>:/W4>
	)

target_compile_options(EDOPro PRIVATE ${EDOPRO_COMPILE_OPTIONS})
target_compile_options(EDOProStatic PRIVATE ${EDOPRO_COMPILE_OPTIONS})

if(EDOPRO_MSVC_STATIC AND MSVC)
	set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
	foreach(CompilerFlag ${CompilerFlags})
  		string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
	endforeach()
	# CMake 3.15
	# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

cmake_minimum_required(VERSION 3.12)
project(Hive C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PROJECT_NAME "Hive")
set(PROJECT_VERSION_MAJOR "0")
set(PROJECT_VERSION_MINOR "1")
set(PROJECT_VERSION_PATCH "0")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fPIC -O0 -Wall -pedantic -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fPIC -Wall -pedantic -Wextra")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fPIC -O3 -g -Wall -pedantic -Wextra")

set(CMAKE_BUILD_TYPE RelWithDebInfo)

option(BUILD_SERVER "Build hive library and metadata/storage server" OFF)
option(BUILD_CLIENT "Build hive client library and examples" ON)

include_directories(src thirdparty/include thirdparty/persistent)

###############################################################################
### Build hive server
###############################################################################
if(BUILD_SERVER)
find_library(LIB_THRIFT libthrift.so)
if (LIB_THRIFT)
		message("Found libthrift.so: ${LIB_THRIFT}")
else()
		message(FATAL_ERROR "libthrift.so not found.")
endif()

find_library(LIB_THRIFTNB libthriftnb.so)
if (LIB_THRIFTNB)
		message("Found libthriftnb.so: ${LIB_THRIFTNB}")
else()
		message(FATAL_ERROR "libthriftnb.so not found.")
endif()

find_library(LIB_EVENT libevent.so)
if (LIB_EVENT)
		message("Found libevent.so: ${LIB_EVENT}")
else()
		message(FATAL_ERROR "libevent.so not found.")
endif()

# use nonblocking storage server, otherwise, use threaded storage server(i.e. one thread per connection)
add_definitions(-DUSE_NONBLOCKING_STORAGE_SERVER)

file(GLOB LIBHIVE_SOURCES
    "src/metadata/block/*"
    "src/metadata/blob/*"
    "src/storage/*"
    "src/monitor/*"
    "src/predictor/*"
    "src/workflow/*"
    "src/utils/*")
file (GLOB METADATA_SERVER_SOURCES
    "src/server/metadata_server.cpp")
file (GLOB STORAGE_SERVER_SOURCES
    "src/server/storage_server.cpp")
  
add_library(hive SHARED ${LIBHIVE_SOURCES})
target_link_libraries(hive PRIVATE ${LIB_THRIFT} ${LIB_THRIFTNB} ${LIB_EVENT})

add_executable(metadata_server ${METADATA_SERVER_SOURCES})
target_link_libraries(metadata_server PRIVATE hive pthread)

add_executable(storage_server ${STORAGE_SERVER_SOURCES})
target_link_libraries(storage_server PRIVATE hive pthread)
endif()

###############################################################################
### Build hive client
###############################################################################
if(BUILD_CLIENT)

file(GLOB LIBGGS3_SOURCES "thirdparty/persistent/net/*")

find_library(LIB_THRIFT_STATIC libthrift.a)
if (LIB_THRIFT_STATIC)
		message("Found libthrift.a: ${LIB_THRIFT_STATIC}")
		add_library( thrift_static STATIC IMPORTED)
		set_target_properties( thrift_static PROPERTIES IMPORTED_LOCATION ${LIB_THRIFT_STATIC} )
else()
		message(FATAL_ERROR "libthrift.a not found.")
endif()

find_library(LIB_THRIFTNB_STATIC libthriftnb.a)
if (LIB_THRIFTNB_STATIC)
		message("Found libthriftnb.a: ${LIB_THRIFTNB_STATIC}")
		add_library( thriftnb_static STATIC IMPORTED)
		set_target_properties( thriftnb_static PROPERTIES IMPORTED_LOCATION ${LIB_THRIFTNB_STATIC} )
else()
		message(FATAL_ERROR "libthriftnb.a not found.")
endif()

find_library(LIB_EVENT_STATIC libevent.a)
if (LIB_EVENT_STATIC)
		message("Found libevent.a: ${LIB_EVENT_STATIC}")
		add_library( event_static STATIC IMPORTED)
		set_target_properties( event_static PROPERTIES IMPORTED_LOCATION ${LIB_EVENT_STATIC} )
else()
		message(FATAL_ERROR "libevent.a not found.")
endif()

find_library(LIB_CRYPTOPP_STATIC libcrypto++.a)
if (LIB_CRYPTOPP_STATIC)
	message("Found libcrypto++.a: ${LIB_CRYPTOPP_STATIC}")
	add_library( cryptopp_static STATIC IMPORTED )
	set_target_properties( cryptopp_static PROPERTIES IMPORTED_LOCATION ${LIB_CRYPTOPP_STATIC} )
else()
	message(FATAL_ERROR "libcrypto++.a not found.")
endif()

find_library(LIB_SSL_STATIC libssl.a)
if (LIB_SSL_STATIC)
	message("Found libssl.a: ${LIB_SSL_STATIC}")
	add_library( ssl_static STATIC IMPORTED )
	set_target_properties( ssl_static PROPERTIES IMPORTED_LOCATION ${LIB_SSL_STATIC} )
else()
	message(FATAL_ERROR "libssl.a not found.")
endif()

find_library(LIB_SSL libssl.so)
if (LIB_SSL)
	message("Found libssl.so: ${LIB_SSL}")
	add_library( ssl_dynamic SHARED IMPORTED )
	set_target_properties( ssl_dynamic PROPERTIES IMPORTED_LOCATION ${LIB_SSL} )
else()
	message(FATAL_ERROR "libssl.so not found.")
endif()

find_library(LIB_CRYPTO_STATIC libcrypto.a)
if (LIB_CRYPTO_STATIC)
	message("Found libcrypto.a: ${LIB_CRYPTO_STATIC}")
	add_library( crypto_static STATIC IMPORTED )
	set_target_properties( crypto_static PROPERTIES IMPORTED_LOCATION ${LIB_CRYPTO_STATIC} )
else()
	message(FATAL_ERROR "libcrypto.a not found.")
endif()

find_library(LIB_CRYPTO libcrypto.so)
if (LIB_CRYPTO)
	message("Found libcrypto.so: ${LIB_CRYPTO}")
	add_library( crypto_dynamic SHARED IMPORTED )
	set_target_properties( crypto_dynamic PROPERTIES IMPORTED_LOCATION ${LIB_CRYPTO} )
else()
	message(FATAL_ERROR "libcrypto.so not found.")
endif()

find_library(LIB_DL_STATIC libdl.a)
if (LIB_DL_STATIC)
	message("Found libdl.a: ${LIB_DL_STATIC}")
	add_library( dl_static STATIC IMPORTED )
	set_target_properties( dl_static PROPERTIES IMPORTED_LOCATION ${LIB_DL_STATIC} )
else()
	message(FATAL_ERROR "libdl.a not found.")
endif()

find_library(LIB_DL libdl.so)
if (LIB_DL)
	message("Found libdl.so: ${LIB_DL}")
	add_library( dl_dynamic SHARED IMPORTED )
	set_target_properties( dl_dynamic PROPERTIES IMPORTED_LOCATION ${LIB_DL} )
else()
	message(FATAL_ERROR "libdl.so not found.")
endif()

find_library(LIB_GGS3_STATIC libggs3.a)
if (LIB_GGS3_STATIC)
	message("Found libggs3.a: ${LIB_GGS3_STATIC}")
	add_library( ggs3_static STATIC IMPORTED )
	set_target_properties( ggs3_static PROPERTIES IMPORTED_LOCATION ${LIB_GGS3_STATIC} )
else()
	message("libggs3.a not found, will be built from ${PROJECT_SOURCE_DIR}/thirdparty/persistent/net")
	add_library( ggs3_static STATIC ${LIBGGS3_SOURCES} )
endif()

# enable cache storage client for different slices in the same storage server
add_definitions(-DENABLE_CACHE_CLIENT)
# use nonblocking storage server, otherwise, use threaded storage server(i.e. one thread per connection)
add_definitions(-DUSE_NONBLOCKING_STORAGE_SERVER)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")
# set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc -pthread")
set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc -static -lrt -Wl,--whole-archive -lpthread -Wl,--no-whole-archive")

file (GLOB LIBHIVECLIENT_SOURCES
				"src/client/*"
				"src/metadata/blob/inode_management_client.h"
				"src/metadata/blob/inode_management_client.cpp"
				"src/metadata/blob/inode_management_service_types.tcc"
				"src/metadata/blob/inode_management_service_types.h"
				"src/metadata/blob/inode_management_service_types.cpp"
				"src/metadata/blob/inode_management_service.tcc"
				"src/metadata/blob/inode_management_service.h"
				"src/metadata/blob/inode_management_service.cpp"
				"src/storage/storage_management_client.h"
				"src/storage/storage_management_client.cpp"
				"src/storage/storage_management_service_types.tcc"
				"src/storage/storage_management_service_types.h"
				"src/storage/storage_management_service_types.cpp"
				"src/storage/storage_management_service.tcc"
				"src/storage/storage_management_service.h"
				"src/storage/storage_management_service.cpp"
				)

        add_library(hiveclient STATIC ${LIBHIVECLIENT_SOURCES})
        target_link_libraries(hiveclient PRIVATE thrift_static thriftnb_static event_static
                                              ggs3_static
                                              cryptopp_static
                                              ssl_static crypto_static
											# dl pthread)
                                              dl_static)
        set_target_properties(hiveclient PROPERTIES OUTPUT_NAME hiveclient)
endif()
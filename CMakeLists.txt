cmake_minimum_required(VERSION 3.0)
project(libaux)

include_directories("src")

file(GLOB LIBAUX_HEADERS "src/*.hpp")
file(GLOB LIBAUX_SRC "src/*.cpp")

add_library(aux STATIC ${LIBAUX_SRC})
add_library(aux_shared SHARED ${LIBAUX_SRC})
set_target_properties(aux_shared PROPERTIES OUTPUT_NAME aux)

install(FILES ${LIBAUX_HEADERS} DESTINATION include/aux)

install(TARGETS aux aux_shared
        ARCHIVE DESTINATION lib/aux
        LIBRARY DESTINATION lib/aux
        PUBLIC_HEADER DESTINATION include/aux
)

set(LIBAUX_DESCRIPTION "Auxiliary data structures and batch allocation library")

SET(CPACK_SET_DESTDIR "on")
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_NAME "aux-dev")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${LIBAUX_DESCRIPTION}")
set(CPACK_DEBIAN_FILE_NAME "${CPACK_DEBIAN_PACKAGE_NAME}.${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}_amd64.deb")
set(CPACK_PACKAGE_DESCRIPTION "${LIBAUX_DESCRIPTION}")
set(CPACK_PACKAGE_CONTACT "Alexander Golenev alex.golenev@gmail.com")

include(CPack)


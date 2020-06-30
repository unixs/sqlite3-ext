set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE  ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(OpenMP REQUIRED)

pkg_check_modules(JANSSON REQUIRED jansson)
pkg_check_modules(SQLITE REQUIRED sqlite3)
pkg_check_modules(JEMALLOC REQUIRED jemalloc)

add_library(actionlog_to_json
        SHARED
        actionlog_to_json.c)
set_target_properties(actionlog_to_json
        PROPERTIES
        PREFIX "")
target_include_directories(actionlog_to_json
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${GLIB2_INCLUDE_DIRS}
        ${JANSSON_INCLUDE_DIRS}
        ${SQLITE_INCLUDE_DIRS}
        ${JEMALLOC_INCLUDE_DIRS})
target_link_libraries(actionlog_to_json
        PRIVATE
        OpenMP::OpenMP_C
        ${GLIB2_LDFLAGS}
        ${JANSSON_LDFLAGS}
        ${SQLITE_LDFLAGS}
        ${JEMALLOC_LDFLAGS})

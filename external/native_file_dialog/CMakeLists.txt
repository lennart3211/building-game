set(TARGET_NAME nfd)

set(PUBLIC_HEADER_FILES
        include/nfd.h
        include/nfd.hpp
        include/nfd_sdl2.h
        include/nfd_glfw3.h)

set(SOURCE_FILES ${PUBLIC_HEADER_FILES})

if(WIN32)
  list(APPEND SOURCE_FILES nfd_win.cpp)
elseif(UNIX AND NOT APPLE)
  find_package(PkgConfig REQUIRED)
  option(NFD_PORTAL "Use xdg-desktop-portal instead of GTK" OFF)
  if(NOT NFD_PORTAL)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    message("Using GTK version: ${GTK3_VERSION}")
    list(APPEND SOURCE_FILES nfd_gtk.cpp)
  else()
    pkg_check_modules(DBUS REQUIRED dbus-1)
    message("Using DBUS version: ${DBUS_VERSION}")
    list(APPEND SOURCE_FILES nfd_portal.cpp)
  endif()
elseif(APPLE)
  find_library(APPKIT_LIBRARY AppKit)
  list(APPEND SOURCE_FILES nfd_cocoa.m)
endif()

add_library(${TARGET_NAME} STATIC ${SOURCE_FILES})

target_include_directories(${TARGET_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if(UNIX AND NOT APPLE)
  if(NOT NFD_PORTAL)
    target_include_directories(${TARGET_NAME} PRIVATE ${GTK3_INCLUDE_DIRS})
    target_link_libraries(${TARGET_NAME} PRIVATE ${GTK3_LIBRARIES})
  else()
    target_include_directories(${TARGET_NAME} PRIVATE ${DBUS_INCLUDE_DIRS})
    target_link_libraries(${TARGET_NAME} PRIVATE ${DBUS_LIBRARIES})
    target_compile_definitions(${TARGET_NAME} PUBLIC NFD_PORTAL)
  endif()
endif()

if(APPLE)
  target_link_libraries(${TARGET_NAME} PRIVATE ${APPKIT_LIBRARY})
endif()

set_target_properties(${TARGET_NAME} PROPERTIES
        PUBLIC_HEADER "${PUBLIC_HEADER_FILES}"
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Explicitly set the output directory
set_target_properties(${TARGET_NAME} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

if(NFD_INSTALL)
  include(GNUInstallDirs)
  install(TARGETS ${TARGET_NAME}
          EXPORT ${TARGET_NAME}-export
          ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
          PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
  install(EXPORT ${TARGET_NAME}-export
          DESTINATION lib/cmake/${TARGET_NAME}
          NAMESPACE ${TARGET_NAME}::
          FILE ${TARGET_NAME}-config.cmake
  )
endif()
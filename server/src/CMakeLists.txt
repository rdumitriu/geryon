
PROJECT (yahsrv)

set (GERYON_VERSION_MAJOR 0)
set (GERYON_VERSION_MINOR 1)
set (GERYON_VERSION_BUILDNO 201402-1)

CONFIGURE_FILE (
  "${PROJECT_SOURCE_DIR}/../include/server_build.hpp.in"
  "${PROJECT_BINARY_DIR}/../include/server_build.hpp"
)

SET (geryon_SRC
     main.cpp
     )

ADD_EXECUTABLE(geryon ${geryon_SRC})

INCLUDE_DIRECTORIES ("../include" "../../commons/include" "${PROJECT_BINARY_DIR}/../include/")

TARGET_LINK_LIBRARIES(geryon ${LIBS} ${AGRADE_LIBS})

INSTALL(TARGETS geryon DESTINATION "${AGRADE_INSTALL_BIN}")

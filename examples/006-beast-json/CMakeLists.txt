
set(EXEC_NAME beast-json)
file(GLOB BIN_SOURCES main.cpp app.hpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(${EXEC_NAME} ${BIN_SOURCES})
target_link_libraries(${EXEC_NAME}
        PRIVATE ${LIB_NAME}
        )
set_target_properties(${EXEC_NAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)
#install(TARGETS ${EXEC_NAME} DESTINATION /srv/http/cgi-bin)

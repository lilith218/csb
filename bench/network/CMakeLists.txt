file(GLOB AGENTS CONFIGURE_DEPENDS *.c)

foreach(AGENT ${AGENTS})
    message(STATUS "Adding agent ${AGENT}")
    get_filename_component(AGENT_NAME ${AGENT} NAME_WLE)
    add_executable(${AGENT_NAME} ${AGENT})
    # TODO: if required
    target_link_libraries(${AGENT_NAME} hiredis)
endforeach(AGENT)
# Compile client




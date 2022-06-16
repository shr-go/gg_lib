# example part
set(ExampleSrc
        simple/LoggingExample.cc
        simple/AsyncLogging.cc
        )

foreach(File IN LISTS ExampleSrc)
    get_filename_component(ExeName ${File} NAME_WE)
    add_executable(${ExeName} ${File})
    target_link_libraries(${ExeName} gg_lib)
endforeach()

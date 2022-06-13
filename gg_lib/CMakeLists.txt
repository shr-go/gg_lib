set(GG_LIB_SRC
        Timestamp.cc
        FileUtil.cc
        LogStream.cc)

add_library(gg_lib ${GG_LIB_SRC})
target_link_libraries(gg_lib pthread rt)
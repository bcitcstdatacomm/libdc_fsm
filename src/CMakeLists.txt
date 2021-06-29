# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

set(HEADER_LIST
        "${libdc_fsm_SOURCE_DIR}/include/dc_fsm/fsm.h"
        )

set(SOURCE_LIST
        "fsm.c"
        )

# Make an static library
add_library(dc_fsm SHARED ${SOURCE_LIST} ${HEADER_LIST})
target_include_directories(dc_fsm PRIVATE ../include/dc_fsm)
target_include_directories(dc_fsm PRIVATE /usr/local/include)
target_link_directories(dc_fsm PRIVATE /usr/local/lib)

# All users of this library will need at least C11
target_compile_features(dc_fsm PUBLIC c_std_11)
target_compile_options(dc_fsm PRIVATE -fstack-protector-all -ftrapv)
target_compile_options(dc_fsm PRIVATE -Wpedantic -Wall -Wextra)
target_compile_options(dc_fsm PRIVATE -Wdouble-promotion -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wnull-dereference -Winit-self -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wunused-local-typedefs -Wstrict-overflow=5 -Wmissing-noreturn -Walloca -Wfloat-equal -Wdeclaration-after-statement -Wshadow -Wpointer-arith -Wabsolute-value -Wundef -Wexpansion-to-defined -Wunused-macros -Wno-endif-labels -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wdangling-else -Wdate-time -Wempty-body -Wsign-conversion -Wfloat-conversion -Waggregate-return -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-declarations -Wpacked -Wpadded -Wredundant-decls -Wnested-externs -Winline -Winvalid-pch -Wlong-long -Wvariadic-macros -Wdisabled-optimization -Wstack-protector -Woverlength-strings )

find_library(LIBDC_POSIX dc_posix REQUIRED)
if(LIBDC_POSIX)
    target_link_libraries(dc_fsm ${LIBDC_POSIX})
endif()

install(TARGETS dc_fsm LIBRARY DESTINATION lib)
install(FILES ${HEADER_LIST} DESTINATION include/dc_fsm)

# IDEs should put the headers in a nice place
source_group(
        TREE "${PROJECT_SOURCE_DIR}/include"
        PREFIX "Header Files"
        FILES ${HEADER_LIST}
)

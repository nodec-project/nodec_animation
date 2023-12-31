include(FetchContent)

function(add_target_if_missing TARGET GIT_REPOSITORY GIT_TAG)
    if(TARGET ${TARGET})
        return()
    endif()

    message("${TARGET} not found. Fetch the contents")
    FetchContent_Declare(
        ${TARGET}
        GIT_REPOSITORY ${GIT_REPOSITORY}
        GIT_TAG ${GIT_TAG}
        GIT_SHALLOW 1
    )
    FetchContent_MakeAvailable(${TARGET})
endfunction(add_target_if_missing)

set(NODEC_DIR ../../../nodec)
set(MODULE_DIR ../..)

add_target_if_missing(nodec https://github.com/nodec-project/nodec.git main)
add_target_if_missing(nodec_scene https://github.com/nodec-project/nodec_scene.git main)
add_target_if_missing(nodec_serialization https://github.com/nodec-project/nodec_serialization.git main)
add_target_if_missing(nodec_scene_serialization https://github.com/nodec-project/nodec_scene_serialization.git main)

function(add_basic_test TARGET TEST_SOURCES)
    add_executable(${TARGET} ${TEST_SOURCES})
    target_link_libraries(${TARGET} nodec_animation)
    target_include_directories(${TARGET} PRIVATE ${nodec_SOURCE_DIR}/tests/common)

    add_test(NAME ${TARGET} COMMAND ${TARGET})
endfunction(add_basic_test)

add_basic_test("animation_curve" animation_curve.cpp)
add_basic_test("animation_clip" animation_clip.cpp)
add_basic_test("animated_component_writer" animated_component_writer.cpp)

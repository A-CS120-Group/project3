cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(JUCE)

set(NODE1 "Node1")
set(NODE2 "Node2")
set(NODE3 "Node3")

macro(setup name)
    project(${name} VERSION 0.0.1)
    juce_add_gui_app(${name} PRODUCT_NAME ${name})
    juce_generate_juce_header(${name})
    target_link_libraries(${name}
            PRIVATE
            juce::juce_analytics
            juce::juce_audio_basics
            juce::juce_audio_devices
            juce::juce_audio_formats
            juce::juce_audio_plugin_client
            juce::juce_audio_processors
            juce::juce_audio_utils
            juce::juce_box2d
            juce::juce_core
            juce::juce_cryptography
            juce::juce_data_structures
            juce::juce_dsp
            juce::juce_events
            juce::juce_graphics
            juce::juce_gui_basics
            juce::juce_gui_extra
            juce::juce_opengl
            juce::juce_osc
            juce::juce_product_unlocking
            juce::juce_video
            PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)
    target_sources(${name}
            PRIVATE
            include/reader.h
            include/writer.h
            include/utils.cpp
            include/utils.h
            include/main.cpp
            )
endmacro(setup)

setup(${NODE1})
setup(${NODE2})
setup(${NODE3})
target_sources(${NODE1}
        PRIVATE
        Node1/Node.h
        )
target_sources(${NODE2}
        PRIVATE
        Node2/Node.h
        )
target_sources(${NODE3}
        PRIVATE
        Node3/Node.h
        )

include_directories(${Boost_INCLUDE_DIR})

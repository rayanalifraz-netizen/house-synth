cmake_minimum_required(VERSION 3.22)
project(HouseSynth VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(${CMAKE_SOURCE_DIR}/../JUCE JUCE)

juce_add_plugin(HouseSynth
    COMPANY_NAME "RayanFraz"
    PLUGIN_MANUFACTURER_CODE Rfaz
    PLUGIN_CODE Hous
    FORMATS VST3 AU
    PRODUCT_NAME "House Synth"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES "Instrument|Synth"
)

juce_generate_juce_header(HouseSynth)

target_sources(HouseSynth PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/SynthVoice.cpp
)

target_compile_definitions(HouseSynth PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(HouseSynth PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_audio_plugin_client
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

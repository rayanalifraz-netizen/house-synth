cmake_minimum_required(VERSION 3.22)
project(HousePlugins VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(${CMAKE_SOURCE_DIR}/../JUCE JUCE)

# ── Helper macro ─────────────────────────────────────────────────────────────
macro(house_plugin TARGET CODE NAME SOURCES)
    juce_add_plugin(${TARGET}
        COMPANY_NAME "RayanFraz"
        PLUGIN_MANUFACTURER_CODE Rfaz
        PLUGIN_CODE ${CODE}
        FORMATS VST3 AU
        PRODUCT_NAME ${NAME}
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        VST3_CATEGORIES "Instrument|Synth"
    )
    juce_generate_juce_header(${TARGET})
    target_sources(${TARGET} PRIVATE ${SOURCES})
    target_compile_definitions(${TARGET} PUBLIC
        JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
    target_link_libraries(${TARGET} PRIVATE
        juce::juce_audio_utils juce::juce_dsp juce::juce_audio_plugin_client
        PUBLIC juce::juce_recommended_config_flags
               juce::juce_recommended_lto_flags
               juce::juce_recommended_warning_flags)
endmacro()

# ── Plugins ──────────────────────────────────────────────────────────────────
house_plugin(HouseSynth Hous "House Synth"
    "Source/PluginProcessor.cpp;Source/PluginEditor.cpp;Source/SynthVoice.cpp")

house_plugin(HouseStab Hstb "House Stab"
    "StabSource/StabProcessor.cpp;StabSource/StabEditor.cpp;StabSource/StabVoice.cpp")

house_plugin(SupersawPad Hssw "Supersaw Pad"
    "SupersawSource/SupersawProcessor.cpp;SupersawSource/SupersawEditor.cpp;SupersawSource/SupersawVoice.cpp")

house_plugin(HousePiano Hpno "House Piano"
    "PianoSource/PianoProcessor.cpp;PianoSource/PianoEditor.cpp;PianoSource/PianoVoice.cpp")

cmake_minimum_required(VERSION 3.15)
project(winbolo_client C)

set(CMAKE_C_STANDARD 17) # Seems to work.

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This project is currently 32-bit only.")
endif()

add_library(zlib STATIC
    winbolo/src/zlib/inflate.c
    winbolo/src/zlib/deflate.c
    winbolo/src/zlib/trees.c
    winbolo/src/zlib/inftrees.c
    winbolo/src/zlib/inffast.c
    winbolo/src/zlib/crc32.c
    winbolo/src/zlib/zutil.c
    winbolo/src/zlib/adler32.c
    winbolo/src/zlib/zip.c
    winbolo/src/zlib/unzip.c
    winbolo/src/zlib/ioapi.c
    winbolo/src/zlib/compress.c
)

file(GLOB CLIENT_SOURCES
    winbolo/src/bolo/allience.c winbolo/src/bolo/backend.c winbolo/src/bolo/bases.c winbolo/src/bolo/bolo_map.c
    winbolo/src/bolo/building.c winbolo/src/bolo/crc.c winbolo/src/bolo/debug_file_output.c
    winbolo/src/bolo/explosions.c winbolo/src/bolo/floodfill.c winbolo/src/bolo/gametype.c winbolo/src/bolo/global.c
    winbolo/src/bolo/grass.c winbolo/src/bolo/labels.c winbolo/src/bolo/lgm.c winbolo/src/bolo/log.c
    winbolo/src/bolo/mathWinbolo.c winbolo/src/bolo/messages.c winbolo/src/bolo/mines.c winbolo/src/bolo/minesexp.c 
    winbolo/src/bolo/netmt.c winbolo/src/bolo/netplayers.c winbolo/src/bolo/netpnb.c winbolo/src/bolo/network.c
    winbolo/src/bolo/pillbox.c winbolo/src/bolo/players.c winbolo/src/bolo/playersrejoin.c winbolo/src/bolo/rubble.c
    winbolo/src/bolo/screen.c winbolo/src/bolo/screenbrainmap.c winbolo/src/bolo/screenbullet.c
    winbolo/src/bolo/screencalc.c winbolo/src/bolo/screenlgm.c winbolo/src/bolo/screentank.c winbolo/src/bolo/scroll.c
    winbolo/src/bolo/shells.c winbolo/src/bolo/sounddist.c winbolo/src/bolo/starts.c winbolo/src/bolo/swamp.c
    winbolo/src/bolo/tank.c winbolo/src/bolo/tankexp.c winbolo/src/bolo/treegrow.c winbolo/src/bolo/udppackets.c
    winbolo/src/bolo/util.c
    winbolo/src/bsd/*.c
    winbolo/src/gui/*.c
    winbolo/src/gui/win32/clientmutex.c winbolo/src/gui/win32/dialogAlliance.c winbolo/src/gui/win32/dnslookups.c
    winbolo/src/gui/win32/winbolo.c winbolo/src/gui/win32/brainsHandler.c winbolo/src/gui/win32/cuursor.c
    winbolo/src/gui/win32/dialogAbout.c winbolo/src/gui/win32/dialogGameInfo.c winbolo/src/gui/win32/dialogGameFinder.c
    winbolo/src/gui/win32/dialogGameSetup.c winbolo/src/gui/win32/dialogKeySetup.c
    winbolo/src/gui/win32/dialogLanguages.c winbolo/src/gui/win32/dialogMessages.c
    winbolo/src/gui/win32/dialogNetworkInfo.c winbolo/src/gui/win32/dialogOpening.c
    winbolo/src/gui/win32/dialogPassword.c winbolo/src/gui/win32/dialogSetName.c winbolo/src/gui/win32/dialogSkins.c
    winbolo/src/gui/win32/dialogSystemInfo.c winbolo/src/gui/win32/dialogTrackerSetup.c
    winbolo/src/gui/win32/dialogUDPSetup.c winbolo/src/gui/win32/dialogWinbolonet.c winbolo/src/gui/win32/winutil.c
    winbolo/src/gui/win32/draw.c winbolo/src/gui/win32/Font.c winbolo/src/gui/win32/gamefront.c
    winbolo/src/gui/win32/currentgames.c winbolo/src/gui/win32/input.c winbolo/src/gui/win32/lang.c
    winbolo/src/gui/win32/mappreview.c winbolo/src/gui/win32/netclient.c winbolo/src/gui/win32/sound.c
    winbolo/src/gui/win32/dsutil.c
    winbolo/src/lzw/*.c
    winbolo/src/server/servercore.c
    winbolo/src/server/servermessages.c
    winbolo/src/server/servernet.c
    winbolo/src/server/servertransport.c
    winbolo/src/server/threads.c
    winbolo/src/server/BigDigits/*.c
    winbolo/src/winbolonet/*.c
)

# TODO: handle langdll/
#file(GLOB LANG_SOURCES
#    langdll/*.c
#)

add_executable(winbolo
    ${CLIENT_SOURCES}
)

target_sources(winbolo PRIVATE winbolo/src/gui/win32/Resource.rc)

set(ZLIB_LIB "${CMAKE_BINARY_DIR}/zlib.lib")

set_target_properties(winbolo PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# Link Windows libraries
target_link_libraries(winbolo PRIVATE
    ws2_32
    winmm
    comdlg32
    gdi32
    ole32
    shell32
    user32
    ddraw
    dsound
    dinput8
    ${ZLIB_LIB}
)
target_link_options(winbolo PRIVATE "/SUBSYSTEM:WINDOWS,10.0")

# Brain Template.brn

file(GLOB BRAINTEMPLATE_SOURCES
    brains/braintemplate/*.c
)

add_library(BrainTemplate MODULE
    ${BRAINTEMPLATE_SOURCES}
)
target_sources(BrainTemplate PRIVATE brains/braintemplate/brain.rc)

set_target_properties(BrainTemplate PROPERTIES
    OUTPUT_NAME "Brain Template"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Brains"
    SUFFIX ".brn"
)

# make default calling conv stdcall, and keep undecorated export name
if (MSVC)
  target_compile_options(BrainTemplate PRIVATE /Gz)
  target_link_options(BrainTemplate PRIVATE "/EXPORT:BrainMain=_BrainMain@4")
endif()

# Ryan2.brn

file(GLOB RYAN2_SOURCES
    brains/ryan2/*.c
)

add_library(Ryan2 MODULE
    ${RYAN2_SOURCES}
)
target_sources(Ryan2 PRIVATE brains/ryan2/brain.rc)

set_target_properties(Ryan2 PROPERTIES
    OUTPUT_NAME "Ryan2"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Brains"
    SUFFIX ".brn"
)

target_link_libraries(Ryan2 PRIVATE
    ws2_32
)

# make default calling conv stdcall, and keep undecorated export name
if (MSVC)
  target_compile_options(Ryan2 PRIVATE /Gz)
  target_link_options(Ryan2 PRIVATE "/EXPORT:BrainMain=_BrainMain@4")
endif()

# Standard Autopilo.brn

file(GLOB STANDARD_AUTOPILOT_SOURCES
    brains/standardautopilot/standardAutopilot.c
    brains/standardautopilot/FixMath.c
)

add_library(StandardAutopilot MODULE
    ${STANDARD_AUTOPILOT_SOURCES}
)
target_sources(StandardAutopilot PRIVATE brains/standardautopilot/brain.rc)

set_target_properties(StandardAutopilot PROPERTIES
    OUTPUT_NAME "Standard Autopilot"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Brains"
    SUFFIX ".brn"
)

# make default calling conv stdcall, and keep undecorated export name
if (MSVC)
  target_compile_options(StandardAutopilot PRIVATE /Gz)
  target_link_options(StandardAutopilot PRIVATE "/EXPORT:BrainMain=_BrainMain@4")
endif()


# BoloSounds.bsd (DLL containing only WAV resources)

add_library(BoloSounds MODULE)
target_sources(BoloSounds PRIVATE BoloSounds.rc)
set_target_properties(BoloSounds PROPERTIES
    SUFFIX ".bsd"
    LINKER_LANGUAGE C
)
target_link_options(BoloSounds PRIVATE "/NOENTRY")


cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

if(NOT TARGET score_lib_base)
  include(ScoreExternalAddon)
endif()

if(NOT TARGET score_plugin_avnd)
  return()
endif()

project(score_addon_puara LANGUAGES CXX)

set(PUARA_GESTURES_ENABLE_TESTING OFF)
set(PUARA_GESTURES_ENABLE_STANDALONE OFF)

add_subdirectory(3rdparty/BioData SYSTEM)
add_subdirectory(3rdparty/extras SYSTEM)
add_subdirectory(3rdparty/puara-gestures SYSTEM)
add_dependencies(puara_gestures BioData)
add_dependencies(puara_gestures extras)
target_link_libraries(puara_gestures PUBLIC BioData)
target_link_libraries(puara_gestures PUBLIC extras)
#CHANGED

avnd_score_plugin_init(
  BASE_TARGET score_addon_puara
)

target_link_libraries(score_addon_puara PRIVATE puara_gestures)

avnd_score_plugin_add(
  BASE_TARGET score_addon_puara
  SOURCES
    Puara/GestureRecognizer.hpp
    Puara/GestureRecognizer.cpp
  TARGET puara_gesture
  MAIN_CLASS GestureRecognizer
  NAMESPACE puara_gestures::objects
)
#
#
avnd_score_plugin_add(
  BASE_TARGET score_addon_puara
  SOURCES
    Puara/BioDataHeart.hpp
    Puara/BioDataHeart.cpp
    3rdparty/BioData/src/Heart.cpp
    3rdparty/BioData/src/Heart.h
    3rdparty/BioData/src
  TARGET puara_heart
  MAIN_CLASS BioData_Heart
  NAMESPACE puara_gestures::objects
)

avnd_score_plugin_add(
  BASE_TARGET score_addon_puara
  SOURCES
    Puara/BioDataSkinConductance.hpp
    Puara/BioDataSkinConductance.cpp
    3rdparty/BioData/src/SkinConductance.cpp
    3rdparty/BioData/src/SkinConductance.h
    3rdparty/BioData/src
  TARGET puara_skin_conductance
  MAIN_CLASS BioData_Skin_Conductance
  NAMESPACE puara_gestures::objects
)

avnd_score_plugin_add(
  BASE_TARGET score_addon_puara
  SOURCES
    Puara/PeakDetection.hpp
    Puara/PeakDetection.cpp
    3rdparty/extras/PeakDetector.cpp
    3rdparty/extras/PeakDetector.h
  TARGET puara_peak_detection
  MAIN_CLASS PeakDetection
  NAMESPACE puara_gestures::objects
)
#
# avnd_score_plugin_add(
#   BASE_TARGET score_addon_puara
#   SOURCES
#     Puara/Roll.hpp
#     Puara/Roll.cpp
#   TARGET puara_roll
#   MAIN_CLASS Roll
#   NAMESPACE puara_gestures::objects
# )
#
# avnd_score_plugin_add(
#   BASE_TARGET score_addon_puara
#   SOURCES
#     Puara/Tilt.hpp
#     Puara/Tilt.cpp
#   TARGET puara_tilt
#   MAIN_CLASS Tilt
#   NAMESPACE puara_gestures::objects
# )
#
# avnd_score_plugin_add(
#   BASE_TARGET score_addon_puara
#   SOURCES
#     Puara/Jab.hpp
#     Puara/Jab.cpp
#   TARGET puara_jab
#   MAIN_CLASS Jab
#   NAMESPACE puara_gestures::objects
# )
#
# avnd_score_plugin_add(
#   BASE_TARGET score_addon_puara
#   SOURCES
#     Puara/Shake.hpp
#     Puara/Shake.cpp
#   TARGET puara_shake
#   MAIN_CLASS Shake
#   NAMESPACE puara_gestures::objects
# )

avnd_score_plugin_finalize(
  BASE_TARGET score_addon_puara
  PLUGIN_VERSION 1
  PLUGIN_UUID "049a40e0-01d9-4040-9d00-21f6931c3035"
)

#include "MOEffector.hpp"

#include "mo/Core.hpp"
#include "AEGP_SuiteHandler.h"
#include "AEFX_SuiteHelper.h"
#include <adobesdk/DrawbotSuite.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <new>
#include <vector>

namespace {

PF_Err about(PF_InData* in_data, PF_OutData* outData) {
    PF_SPRINTF(outData->return_msg,
               "MO Effector Native v%d.%d\rSingle-layer procedural cloner.\rBy Bruno Jorri",
               kMajorVersion,
               kMinorVersion);
    return PF_Err_NONE;
}

PF_Err globalSetup(PF_OutData* outData) {
    outData->my_version = PF_VERSION(kMajorVersion,
                                     kMinorVersion,
                                     kBugVersion,
                                     PF_Stage_DEVELOP,
                                     kBuildVersion);
    outData->out_flags = PF_OutFlag_CUSTOM_UI |
                         PF_OutFlag_PIX_INDEPENDENT |
                         PF_OutFlag_USE_OUTPUT_EXTENT |
                         PF_OutFlag_NON_PARAM_VARY |
                         PF_OutFlag_DEEP_COLOR_AWARE;
    outData->out_flags2 = PF_OutFlag2_FLOAT_COLOR_AWARE |
                          PF_OutFlag2_SUPPORTS_SMART_RENDER |
                          PF_OutFlag2_SUPPORTS_THREADED_RENDERING;
    return PF_Err_NONE;
}

PF_Err paramsSetup(PF_InData* in_data, PF_OutData* outData) {
    PF_Err err = PF_Err_NONE;
    PF_ParamDef def;

    AEFX_CLR_STRUCT(def);
    def.flags = PF_ParamFlag_START_COLLAPSED;
    PF_ADD_TOPIC("Source", DISK_SOURCE_TOPIC);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POPUP("Primitive", 3, 1, "Circle|Square|Polygon", DISK_PRIMITIVE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Polygon Sides", 3, 32, 3, 12, 6, DISK_POLYGON_SIDES);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(DISK_SOURCE_TOPIC_END);

    AEFX_CLR_STRUCT(def);
    PF_ADD_TOPIC("Formation", DISK_FORMATION_TOPIC);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POPUP("Mode", 7, 1, "Grid|Linear|Circle|Scatter|Globe 2.5D|Bezier Path|Z Circle",
                 DISK_FORMATION);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Clone Count", 1, 100000, 1, 2000, 24, DISK_CLONE_COUNT);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Rows", 1, 1000, 1, 100, 4, DISK_ROWS);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Columns", 1, 1000, 1, 100, 6, DISK_COLUMNS);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POINT("Center", 50, 50, false, DISK_CENTER);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Spacing X", 0, 10000, 0, 1000, 160, 1, 0, 0, DISK_SPACING_X);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Spacing Y", 0, 10000, 0, 1000, 160, 1, 0, 0, DISK_SPACING_Y);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Radius", 0, 10000, 0, 2000, 320, 1, 0, 0, DISK_RADIUS);

    AEFX_CLR_STRUCT(def);
    def.flags = PF_ParamFlag_START_COLLAPSED;
    PF_ADD_TOPIC("Advanced", DISK_FORMATION_ADVANCED_TOPIC);

    AEFX_CLR_STRUCT(def);
    PF_ADD_ANGLE("Module Rotation", 0, DISK_FORMATION_ROTATION);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Circle Rings", 1, 100, 1, 24, 1, DISK_CIRCLE_RINGS);

    AEFX_CLR_STRUCT(def);
    PF_ADD_ANGLE("Z Circle Tilt", 25, DISK_Z_CIRCLE_TILT);

    AEFX_CLR_STRUCT(def);
    PF_ADD_ANGLE("Z Circle Rotation", 0, DISK_Z_CIRCLE_ROTATION);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Z Circle Speed", -360, 360, -180, 180, 0, 1, 0, 0,
                         DISK_Z_CIRCLE_SPEED);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Z Depth Scale", 0, 100, 0, 100, 80, DISK_Z_CIRCLE_DEPTH);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(DISK_FORMATION_ADVANCED_TOPIC_END);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(DISK_FORMATION_TOPIC_END);

    AEFX_CLR_STRUCT(def);
    PF_ADD_TOPIC("Effectors", DISK_EFFECTORS_TOPIC);

    AEFX_CLR_STRUCT(def);
    def.flags = PF_ParamFlag_START_COLLAPSED;
    PF_ADD_TOPIC("Effector 1", DISK_EFFECTOR_TOPIC);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOX("Enable", "Enable native influence", true, 0, DISK_EFFECTOR_ENABLE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POINT("Effector Center", 50, 50, false, DISK_EFFECTOR_CENTER);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POPUP("Shape", 4, 1, "Circle|Box|Linear X|Linear Y", DISK_EFFECTOR_SHAPE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Outer Radius", 1, 10000, 1, 2000, 300, 1, 0, 0, DISK_EFFECTOR_RADIUS);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Inner Radius", 0, 10000, 0, 2000, 0, 1, 0, 0, DISK_EFFECTOR_INNER_RADIUS);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POPUP("Falloff", 6, 2, "Linear|Smooth|Ease In|Ease Out|Gaussian|Constant", DISK_EFFECTOR_FALLOFF);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Falloff Power", 0.1, 10, 0.1, 10, 2, 1, 0, 0, DISK_EFFECTOR_POWER);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOX("Invert", "Invert influence inside the radius", false, 0, DISK_EFFECTOR_INVERT);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Strength", 0, 100, 0, 100, 100, DISK_EFFECTOR_STRENGTH);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Position X", -10000, 10000, -1000, 1000, 0, 1, 0, 0, DISK_EFFECTOR_POSITION_X);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Position Y", -10000, 10000, -1000, 1000, -100, 1, 0, 0, DISK_EFFECTOR_POSITION_Y);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Scale Amount", -100, 1000, -100, 500, 50, 1, PF_ValueDisplayFlag_PERCENT, 0, DISK_EFFECTOR_SCALE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_ANGLE("Rotation Amount", 45, DISK_EFFECTOR_ROTATION);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Target Opacity", 0, 100, 0, 100, 0, DISK_EFFECTOR_OPACITY);

    AEFX_CLR_STRUCT(def);
    def.flags = PF_ParamFlag_START_COLLAPSED;
    PF_ADD_TOPIC("Color", DISK_EFFECTOR_COLOR_TOPIC);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOX("Enable Color", "Colorize clones inside Effector 1", false, 0,
                    DISK_EFFECTOR_COLOR_ENABLE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_COLOR("Target Color", 255, 64, 26, DISK_EFFECTOR_TARGET_COLOR);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Color Amount", 0, 100, 0, 100, 100, DISK_EFFECTOR_COLOR_AMOUNT);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(DISK_EFFECTOR_COLOR_TOPIC_END);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(DISK_EFFECTOR_TOPIC_END);

    AEFX_CLR_STRUCT(def);
    def.flags = PF_ParamFlag_START_COLLAPSED;
    PF_ADD_TOPIC("Effector 2", DISK_EFFECTOR_2_TOPIC);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOX("Enable", "Enable second native influence", false, 0, DISK_EFFECTOR_2_ENABLE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POINT("Center", 65, 50, false, DISK_EFFECTOR_2_CENTER);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POPUP("Shape", 4, 1, "Circle|Box|Linear X|Linear Y", DISK_EFFECTOR_2_SHAPE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Outer Radius", 1, 10000, 1, 2000, 300, 1, 0, 0,
                         DISK_EFFECTOR_2_RADIUS);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Inner Radius", 0, 10000, 0, 2000, 0, 1, 0, 0,
                         DISK_EFFECTOR_2_INNER_RADIUS);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POPUP("Falloff", 6, 2, "Linear|Smooth|Ease In|Ease Out|Gaussian|Constant",
                 DISK_EFFECTOR_2_FALLOFF);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Falloff Power", 0.1, 10, 0.1, 10, 2, 1, 0, 0,
                         DISK_EFFECTOR_2_POWER);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOX("Invert", "Invert second influence", false, 0, DISK_EFFECTOR_2_INVERT);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Strength", 0, 100, 0, 100, 100, DISK_EFFECTOR_2_STRENGTH);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Position X", -10000, 10000, -1000, 1000, 100, 1, 0, 0,
                         DISK_EFFECTOR_2_POSITION_X);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Position Y", -10000, 10000, -1000, 1000, 0, 1, 0, 0,
                         DISK_EFFECTOR_2_POSITION_Y);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Scale Amount", -100, 1000, -100, 500, 0, 1,
                         PF_ValueDisplayFlag_PERCENT, 0, DISK_EFFECTOR_2_SCALE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_ANGLE("Rotation Amount", 0, DISK_EFFECTOR_2_ROTATION);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Target Opacity", 0, 100, 0, 100, 100, DISK_EFFECTOR_2_OPACITY);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOX("Enable Color", "Colorize with Effector 2", false, 0,
                    DISK_EFFECTOR_2_COLOR_ENABLE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_COLOR("Target Color", 120, 70, 255, DISK_EFFECTOR_2_TARGET_COLOR);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Color Amount", 0, 100, 0, 100, 100, DISK_EFFECTOR_2_COLOR_AMOUNT);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(DISK_EFFECTOR_2_TOPIC_END);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(DISK_EFFECTORS_TOPIC_END);

    AEFX_CLR_STRUCT(def);
    PF_ADD_TOPIC("Appearance", DISK_APPEARANCE_TOPIC);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Clone Size", 1, 5000, 1, 1000, 80, 1, 0, 0, DISK_CLONE_SIZE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_ANGLE("Rotation", 0, DISK_ROTATION);

    AEFX_CLR_STRUCT(def);
    PF_ADD_COLOR("Color", 31, 200, 255, DISK_COLOR);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Opacity", 0, 100, 0, 100, 100, DISK_OPACITY);

    AEFX_CLR_STRUCT(def);
    def.flags = PF_ParamFlag_START_COLLAPSED;
    PF_ADD_TOPIC("Random", DISK_APPEARANCE_RANDOM_TOPIC);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Random Size", 0, 100, 0, 100, 0, DISK_RANDOM_SIZE);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(DISK_APPEARANCE_RANDOM_TOPIC_END);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(DISK_APPEARANCE_TOPIC_END);

    AEFX_CLR_STRUCT(def);
    def.flags = PF_ParamFlag_START_COLLAPSED;
    PF_ADD_TOPIC("Globe 2.5D", DISK_GLOBE_TOPIC);

    AEFX_CLR_STRUCT(def);
    PF_ADD_ANGLE("Rotation X", 0, DISK_GLOBE_ROTATION_X);

    AEFX_CLR_STRUCT(def);
    PF_ADD_ANGLE("Rotation Y", 0, DISK_GLOBE_ROTATION_Y);

    AEFX_CLR_STRUCT(def);
    PF_ADD_ANGLE("Rotation Z", 0, DISK_GLOBE_ROTATION_Z);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Speed X", -360, 360, -180, 180, 0, 1, 0, 0, DISK_GLOBE_SPEED_X);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Speed Y", -360, 360, -180, 180, 0, 1, 0, 0, DISK_GLOBE_SPEED_Y);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Depth Scale", 0, 100, 0, 100, 80, DISK_GLOBE_DEPTH_SCALE);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(DISK_GLOBE_TOPIC_END);

    AEFX_CLR_STRUCT(def);
    def.flags = PF_ParamFlag_START_COLLAPSED;
    PF_ADD_TOPIC("Layer Source", DISK_LAYER_SOURCE_TOPIC);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POPUP("Source Mode", 2, 1, "Internal Shape|Source Layer", DISK_SOURCE_MODE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_LAYER("Source Layer", PF_LayerDefault_NONE, DISK_SOURCE_LAYER);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(DISK_LAYER_SOURCE_TOPIC_END);

    AEFX_CLR_STRUCT(def);
    def.flags = PF_ParamFlag_START_COLLAPSED;
    PF_ADD_TOPIC("Multi-Source", DISK_MULTI_SOURCE_TOPIC);

    AEFX_CLR_STRUCT(def);
    PF_ADD_LAYER("Source Layer 2", PF_LayerDefault_NONE, DISK_SOURCE_LAYER_2);

    AEFX_CLR_STRUCT(def);
    PF_ADD_LAYER("Source Layer 3", PF_LayerDefault_NONE, DISK_SOURCE_LAYER_3);

    AEFX_CLR_STRUCT(def);
    PF_ADD_LAYER("Source Layer 4", PF_LayerDefault_NONE, DISK_SOURCE_LAYER_4);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POPUP("Distribution", 2, 1, "Cycle|Random", DISK_SOURCE_SELECTION);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Source Seed", 0, 100000, 0, 10000, 1, DISK_SOURCE_RANDOM_SEED);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(DISK_MULTI_SOURCE_TOPIC_END);

    AEFX_CLR_STRUCT(def);
    def.flags = PF_ParamFlag_START_COLLAPSED;
    PF_ADD_TOPIC("Bezier Path", DISK_PATH_TOPIC);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POINT("Start", 20, 50, false, DISK_PATH_START);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POINT("Control 1", 35, 20, false, DISK_PATH_CONTROL_1);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POINT("Control 2", 65, 80, false, DISK_PATH_CONTROL_2);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POINT("End", 80, 50, false, DISK_PATH_END);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOX("Align to Path", "Orient clones along the curve", true, 0, DISK_PATH_ALIGN);

    AEFX_CLR_STRUCT(def);
    PF_ADD_ANGLE("Rotation Offset", 0, DISK_PATH_ROTATION_OFFSET);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Position Offset", -100, 100, -100, 100, 0, 1,
                         PF_ValueDisplayFlag_PERCENT, 0, DISK_PATH_POSITION_OFFSET);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(DISK_PATH_TOPIC_END);

    AEFX_CLR_STRUCT(def);
    def.flags = PF_ParamFlag_START_COLLAPSED;
    PF_ADD_TOPIC("Variation", DISK_VARIATION_TOPIC);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Seed", 0, 1000000, 0, 10000, 1, DISK_VARIATION_SEED);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Position X", 0, 10000, 0, 1000, 0, 1, 0, 0, DISK_POSITION_JITTER_X);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Position Y", 0, 10000, 0, 1000, 0, 1, 0, 0, DISK_POSITION_JITTER_Y);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Scale", 0, 100, 0, 100, 0, DISK_SCALE_JITTER);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Rotation", 0, 3600, 0, 360, 0, 1, 0, 0, DISK_ROTATION_JITTER);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Opacity", 0, 100, 0, 100, 0, DISK_OPACITY_JITTER);

    AEFX_CLR_STRUCT(def);
    def.flags = PF_ParamFlag_START_COLLAPSED;
    PF_ADD_TOPIC("Animation", DISK_VARIATION_ANIMATION_TOPIC);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Speed", 0, 20, 0, 10, 0, 2, 0, 0,
                         DISK_VARIATION_SPEED);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(DISK_VARIATION_ANIMATION_TOPIC_END);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(DISK_VARIATION_TOPIC_END);

    AEFX_CLR_STRUCT(def);
    def.flags = PF_ParamFlag_START_COLLAPSED;
    PF_ADD_TOPIC("Color Palette", DISK_PALETTE_TOPIC);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOX("Enable Palette", "Use multiple clone colors", false, 0, DISK_PALETTE_ENABLE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Color Count", 1, 4, 1, 4, 4, DISK_PALETTE_SIZE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_COLOR("Color 2", 255, 70, 150, DISK_PALETTE_COLOR_2);

    AEFX_CLR_STRUCT(def);
    PF_ADD_COLOR("Color 3", 125, 75, 255, DISK_PALETTE_COLOR_3);

    AEFX_CLR_STRUCT(def);
    PF_ADD_COLOR("Color 4", 255, 165, 35, DISK_PALETTE_COLOR_4);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POPUP("Distribution", 2, 1, "Cycle|Random", DISK_PALETTE_SELECTION);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Palette Seed", 0, 1000000, 0, 10000, 1, DISK_PALETTE_SEED);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOX("Tint Sources", "Multiply Source Layer colors by the palette", true, 0,
                    DISK_PALETTE_TINT_SOURCES);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(DISK_PALETTE_TOPIC_END);

    AEFX_CLR_STRUCT(def);
    def.flags = PF_ParamFlag_START_COLLAPSED;
    PF_ADD_TOPIC("Step / Stagger", DISK_STEP_TOPIC);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOX("Enable", "Enable sequential transforms", false, 0, DISK_STEP_ENABLE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Progress", 0, 100, 0, 100, 0, DISK_STEP_PROGRESS);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Falloff", 1, 100, 1, 100, 20, DISK_STEP_FALLOFF);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOX("Reverse", "Run the sequence from the last clone", false, 0, DISK_STEP_REVERSE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Position X", -10000, 10000, -1000, 1000, 0, 1, 0, 0,
                         DISK_STEP_POSITION_X);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Position Y", -10000, 10000, -1000, 1000, -100, 1, 0, 0,
                         DISK_STEP_POSITION_Y);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Scale Amount", -100, 1000, -100, 500, 0, 1,
                         PF_ValueDisplayFlag_PERCENT, 0, DISK_STEP_SCALE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_ANGLE("Rotation Amount", 0, DISK_STEP_ROTATION);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Target Opacity", 0, 100, 0, 100, 100, DISK_STEP_OPACITY);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(DISK_STEP_TOPIC_END);

    AEFX_CLR_STRUCT(def);
    def.flags = PF_ParamFlag_START_COLLAPSED;
    PF_ADD_TOPIC("Individual Wiggle", DISK_WIGGLE_TOPIC);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOX("Enable", "Animate every clone with an individual phase", false, 0,
                    DISK_WIGGLE_ENABLE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOX("Continuous Motion", "Keep generating smooth random movement", true, 0,
                    DISK_WIGGLE_CONTINUOUS);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Speed", 0, 20, 0, 10, 1, 2, 0, 0, DISK_WIGGLE_SPEED);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Position X", 0, 10000, 0, 1000, 0, 1, 0, 0,
                         DISK_WIGGLE_POSITION_X);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Position Y", 0, 10000, 0, 1000, 0, 1, 0, 0,
                         DISK_WIGGLE_POSITION_Y);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Scale", 0, 100, 0, 100, 0, DISK_WIGGLE_SCALE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Rotation", 0, 3600, 0, 360, 0, 1, 0, 0,
                         DISK_WIGGLE_ROTATION);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Opacity", 0, 100, 0, 100, 0, DISK_WIGGLE_OPACITY);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(DISK_WIGGLE_TOPIC_END);

    AEFX_CLR_STRUCT(def);
    def.flags = PF_ParamFlag_START_COLLAPSED;
    PF_ADD_TOPIC("Connections", DISK_CONNECTIONS_TOPIC);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOX("Enable", "Render lines behind the clones", false, 0,
                    DISK_CONNECTIONS_ENABLE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POPUP("Mode", 3, 2, "Sequence|Nearest|Distance", DISK_CONNECTIONS_MODE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Per Clone", 1, 8, 1, 8, 2, DISK_CONNECTIONS_PER_CLONE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Max Distance", 1, 10000, 1, 2000, 300, 1, 0, 0,
                         DISK_CONNECTIONS_MAX_DISTANCE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOX("Close Loop", "Connect the last clone back to the first in Sequence mode",
                    true, 0, DISK_CONNECTIONS_CLOSE_LOOP);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Thickness", 0.1, 100, 0.1, 20, 2, 2, 0, 0,
                         DISK_CONNECTIONS_THICKNESS);

    AEFX_CLR_STRUCT(def);
    PF_ADD_COLOR("Color", 100, 205, 255, DISK_CONNECTIONS_COLOR);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Opacity", 0, 100, 0, 100, 75, DISK_CONNECTIONS_OPACITY);

    AEFX_CLR_STRUCT(def);
    def.flags = PF_ParamFlag_START_COLLAPSED;
    PF_ADD_TOPIC("Line Effector", DISK_LINE_EFFECTOR_TOPIC);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOX("Enable", "Animate connection lines with a spatial effector", false, 0,
                    DISK_LINE_EFFECTOR_ENABLE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POINT("Center", 50, 50, false, DISK_LINE_EFFECTOR_CENTER);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POPUP("Shape", 4, 1, "Circle|Box|Linear X|Linear Y", DISK_LINE_EFFECTOR_SHAPE);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Outer Radius", 1, 10000, 1, 2000, 300, 1, 0, 0,
                         DISK_LINE_EFFECTOR_RADIUS);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Inner Radius", 0, 10000, 0, 2000, 0, 1, 0, 0,
                         DISK_LINE_EFFECTOR_INNER_RADIUS);

    AEFX_CLR_STRUCT(def);
    PF_ADD_POPUP("Falloff", 6, 2, "Linear|Smooth|Ease In|Ease Out|Gaussian|Constant",
                 DISK_LINE_EFFECTOR_FALLOFF);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Falloff Power", 0.1, 10, 0.1, 10, 2, 1, 0, 0,
                         DISK_LINE_EFFECTOR_POWER);

    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOX("Invert", "Invert line influence", false, 0, DISK_LINE_EFFECTOR_INVERT);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Strength", 0, 100, 0, 100, 100, DISK_LINE_EFFECTOR_STRENGTH);

    AEFX_CLR_STRUCT(def);
    PF_ADD_SLIDER("Target Opacity", 0, 100, 0, 100, 0, DISK_LINE_EFFECTOR_OPACITY);

    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("Width Change", -100, 1000, -100, 500, 200, 1,
                         PF_ValueDisplayFlag_PERCENT, 0, DISK_LINE_EFFECTOR_WIDTH);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(DISK_LINE_EFFECTOR_TOPIC_END);

    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC(DISK_CONNECTIONS_TOPIC_END);

    PF_CustomUIInfo customUi{};
    customUi.events = PF_CustomEFlag_LAYER | PF_CustomEFlag_COMP;
    customUi.comp_ui_alignment = PF_UIAlignment_NONE;
    customUi.layer_ui_alignment = PF_UIAlignment_NONE;
    customUi.preview_ui_alignment = PF_UIAlignment_NONE;
    err = in_data->inter.register_ui(in_data->effect_ref, &customUi);

    outData->num_params = PARAM_COUNT;
    return err;
}

float scaleFor(const PF_RationalScale& scale) {
    return scale.den == 0 ? 1.0F : static_cast<float>(scale.num) / static_cast<float>(scale.den);
}

mo::RenderParams makeCoreParams(PF_InData* inData, PF_ParamDef* params[]) {
    const float scaleX = scaleFor(inData->downsample_x);
    const float scaleY = scaleFor(inData->downsample_y);
    const float scaleUniform = std::min(scaleX, scaleY);

    mo::RenderParams coreParams{};
    coreParams.primitive = static_cast<mo::Primitive>(
        std::clamp(params[PARAM_PRIMITIVE]->u.pd.value - 1, 0, 2));
    coreParams.formation = static_cast<mo::Formation>(
        std::clamp(params[PARAM_FORMATION]->u.pd.value - 1, 0, 6));
    coreParams.cloneCount = params[PARAM_CLONE_COUNT]->u.sd.value;
    coreParams.rows = params[PARAM_ROWS]->u.sd.value;
    coreParams.columns = params[PARAM_COLUMNS]->u.sd.value;
    coreParams.polygonSides = params[PARAM_POLYGON_SIDES]->u.sd.value;
    coreParams.centerX = static_cast<float>(FIX_2_FLOAT(params[PARAM_CENTER]->u.td.x_value)) * scaleX;
    coreParams.centerY = static_cast<float>(FIX_2_FLOAT(params[PARAM_CENTER]->u.td.y_value)) * scaleY;
    coreParams.spacingX = static_cast<float>(params[PARAM_SPACING_X]->u.fs_d.value) * scaleX;
    coreParams.spacingY = static_cast<float>(params[PARAM_SPACING_Y]->u.fs_d.value) * scaleY;
    coreParams.radius = static_cast<float>(params[PARAM_RADIUS]->u.fs_d.value) * scaleUniform;
    coreParams.formationRotation = static_cast<float>(
        FIX_2_FLOAT(params[PARAM_FORMATION_ROTATION]->u.ad.value));
    coreParams.circleRings = params[PARAM_CIRCLE_RINGS]->u.sd.value;
    coreParams.zCircleTilt = static_cast<float>(FIX_2_FLOAT(params[PARAM_Z_CIRCLE_TILT]->u.ad.value));
    coreParams.zCircleRotation = static_cast<float>(
        FIX_2_FLOAT(params[PARAM_Z_CIRCLE_ROTATION]->u.ad.value));
    coreParams.zCircleSpeed = static_cast<float>(params[PARAM_Z_CIRCLE_SPEED]->u.fs_d.value);
    coreParams.zCircleDepthScale = static_cast<float>(params[PARAM_Z_CIRCLE_DEPTH]->u.sd.value) / 100.0F;
    const float timeSeconds = inData->time_scale == 0 ? 0.0F :
        static_cast<float>(inData->current_time) / static_cast<float>(inData->time_scale);
    coreParams.timeSeconds = timeSeconds;
    coreParams.globeRotationX = static_cast<float>(FIX_2_FLOAT(params[PARAM_GLOBE_ROTATION_X]->u.ad.value)) +
                                static_cast<float>(params[PARAM_GLOBE_SPEED_X]->u.fs_d.value) * timeSeconds;
    coreParams.globeRotationY = static_cast<float>(FIX_2_FLOAT(params[PARAM_GLOBE_ROTATION_Y]->u.ad.value)) +
                                static_cast<float>(params[PARAM_GLOBE_SPEED_Y]->u.fs_d.value) * timeSeconds;
    coreParams.globeRotationZ = static_cast<float>(FIX_2_FLOAT(params[PARAM_GLOBE_ROTATION_Z]->u.ad.value));
    coreParams.globeDepthScale = static_cast<float>(params[PARAM_GLOBE_DEPTH_SCALE]->u.sd.value) / 100.0F;
    coreParams.pathStartX = static_cast<float>(FIX_2_FLOAT(params[PARAM_PATH_START]->u.td.x_value)) * scaleX;
    coreParams.pathStartY = static_cast<float>(FIX_2_FLOAT(params[PARAM_PATH_START]->u.td.y_value)) * scaleY;
    coreParams.pathControl1X = static_cast<float>(FIX_2_FLOAT(params[PARAM_PATH_CONTROL_1]->u.td.x_value)) * scaleX;
    coreParams.pathControl1Y = static_cast<float>(FIX_2_FLOAT(params[PARAM_PATH_CONTROL_1]->u.td.y_value)) * scaleY;
    coreParams.pathControl2X = static_cast<float>(FIX_2_FLOAT(params[PARAM_PATH_CONTROL_2]->u.td.x_value)) * scaleX;
    coreParams.pathControl2Y = static_cast<float>(FIX_2_FLOAT(params[PARAM_PATH_CONTROL_2]->u.td.y_value)) * scaleY;
    coreParams.pathEndX = static_cast<float>(FIX_2_FLOAT(params[PARAM_PATH_END]->u.td.x_value)) * scaleX;
    coreParams.pathEndY = static_cast<float>(FIX_2_FLOAT(params[PARAM_PATH_END]->u.td.y_value)) * scaleY;
    coreParams.pathAlign = params[PARAM_PATH_ALIGN]->u.bd.value != 0;
    coreParams.pathPositionOffset = static_cast<float>(params[PARAM_PATH_POSITION_OFFSET]->u.fs_d.value) / 100.0F;
    coreParams.pathRotationOffset = static_cast<float>(
        FIX_2_FLOAT(params[PARAM_PATH_ROTATION_OFFSET]->u.ad.value));
    coreParams.cloneSize = static_cast<float>(params[PARAM_CLONE_SIZE]->u.fs_d.value) * scaleUniform;
    coreParams.rotationDegrees = static_cast<float>(FIX_2_FLOAT(params[PARAM_ROTATION]->u.ad.value));
    coreParams.opacity = static_cast<float>(params[PARAM_OPACITY]->u.sd.value) / 100.0F;
    coreParams.variationSeed = static_cast<std::uint32_t>(
        std::max(0, params[PARAM_VARIATION_SEED]->u.sd.value));
    coreParams.positionJitterX = static_cast<float>(params[PARAM_POSITION_JITTER_X]->u.fs_d.value) * scaleX;
    coreParams.positionJitterY = static_cast<float>(params[PARAM_POSITION_JITTER_Y]->u.fs_d.value) * scaleY;
    coreParams.scaleJitter = static_cast<float>(params[PARAM_SCALE_JITTER]->u.sd.value) / 100.0F;
    coreParams.rotationJitter = static_cast<float>(params[PARAM_ROTATION_JITTER]->u.fs_d.value);
    coreParams.opacityJitter = static_cast<float>(params[PARAM_OPACITY_JITTER]->u.sd.value) / 100.0F;
    coreParams.variationSpeed = static_cast<float>(params[PARAM_VARIATION_SPEED]->u.fs_d.value);
    coreParams.appearanceRandomSize = static_cast<float>(params[PARAM_RANDOM_SIZE]->u.sd.value) / 100.0F;
    coreParams.color.red = static_cast<float>(params[PARAM_COLOR]->u.cd.value.red) / PF_MAX_CHAN8;
    coreParams.color.green = static_cast<float>(params[PARAM_COLOR]->u.cd.value.green) / PF_MAX_CHAN8;
    coreParams.color.blue = static_cast<float>(params[PARAM_COLOR]->u.cd.value.blue) / PF_MAX_CHAN8;
    coreParams.color.alpha = 1.0F;
    coreParams.paletteEnabled = params[PARAM_PALETTE_ENABLE]->u.bd.value != 0;
    coreParams.paletteTintsSources = params[PARAM_PALETTE_TINT_SOURCES]->u.bd.value != 0;
    coreParams.paletteSize = params[PARAM_PALETTE_SIZE]->u.sd.value;
    coreParams.palette[0] = coreParams.color;
    const std::array<int, 3> paletteParams{
        PARAM_PALETTE_COLOR_2, PARAM_PALETTE_COLOR_3, PARAM_PALETTE_COLOR_4};
    for (std::size_t index = 0; index < paletteParams.size(); ++index) {
        const auto& color = params[paletteParams[index]]->u.cd.value;
        coreParams.palette[index + 1].red = static_cast<float>(color.red) / PF_MAX_CHAN8;
        coreParams.palette[index + 1].green = static_cast<float>(color.green) / PF_MAX_CHAN8;
        coreParams.palette[index + 1].blue = static_cast<float>(color.blue) / PF_MAX_CHAN8;
        coreParams.palette[index + 1].alpha = 1.0F;
    }
    coreParams.paletteSelection = static_cast<mo::SourceSelection>(
        std::clamp(params[PARAM_PALETTE_SELECTION]->u.pd.value - 1, 0, 1));
    coreParams.paletteSeed = static_cast<std::uint32_t>(
        std::max(0, params[PARAM_PALETTE_SEED]->u.sd.value));
    coreParams.sourceSelection = static_cast<mo::SourceSelection>(
        std::clamp(params[PARAM_SOURCE_SELECTION]->u.pd.value - 1, 0, 1));
    coreParams.sourceRandomSeed = static_cast<std::uint32_t>(
        std::max(0, params[PARAM_SOURCE_RANDOM_SEED]->u.sd.value));
    coreParams.effector.enabled = params[PARAM_EFFECTOR_ENABLE]->u.bd.value != 0;
    coreParams.effector.invert = params[PARAM_EFFECTOR_INVERT]->u.bd.value != 0;
    coreParams.effector.shape = static_cast<mo::EffectorShape>(
        std::clamp(params[PARAM_EFFECTOR_SHAPE]->u.pd.value - 1, 0, 3));
    coreParams.effector.falloff = static_cast<mo::FalloffType>(
        std::clamp(params[PARAM_EFFECTOR_FALLOFF]->u.pd.value - 1, 0, 5));
    coreParams.effector.centerX = static_cast<float>(FIX_2_FLOAT(params[PARAM_EFFECTOR_CENTER]->u.td.x_value)) * scaleX;
    coreParams.effector.centerY = static_cast<float>(FIX_2_FLOAT(params[PARAM_EFFECTOR_CENTER]->u.td.y_value)) * scaleY;
    coreParams.effector.radius = static_cast<float>(params[PARAM_EFFECTOR_RADIUS]->u.fs_d.value) * scaleUniform;
    coreParams.effector.innerRadius = static_cast<float>(params[PARAM_EFFECTOR_INNER_RADIUS]->u.fs_d.value) * scaleUniform;
    coreParams.effector.power = static_cast<float>(params[PARAM_EFFECTOR_POWER]->u.fs_d.value);
    coreParams.effector.strength = static_cast<float>(params[PARAM_EFFECTOR_STRENGTH]->u.sd.value) / 100.0F;
    coreParams.effector.positionX = static_cast<float>(params[PARAM_EFFECTOR_POSITION_X]->u.fs_d.value) * scaleX;
    coreParams.effector.positionY = static_cast<float>(params[PARAM_EFFECTOR_POSITION_Y]->u.fs_d.value) * scaleY;
    coreParams.effector.scaleAmount = static_cast<float>(params[PARAM_EFFECTOR_SCALE]->u.fs_d.value) / 100.0F;
    coreParams.effector.rotationAmount = static_cast<float>(FIX_2_FLOAT(params[PARAM_EFFECTOR_ROTATION]->u.ad.value));
    coreParams.effector.targetOpacity = static_cast<float>(params[PARAM_EFFECTOR_OPACITY]->u.sd.value) / 100.0F;
    coreParams.effector.colorEnabled = params[PARAM_EFFECTOR_COLOR_ENABLE]->u.bd.value != 0;
    const auto& targetColor = params[PARAM_EFFECTOR_TARGET_COLOR]->u.cd.value;
    coreParams.effector.targetColor.red = static_cast<float>(targetColor.red) / PF_MAX_CHAN8;
    coreParams.effector.targetColor.green = static_cast<float>(targetColor.green) / PF_MAX_CHAN8;
    coreParams.effector.targetColor.blue = static_cast<float>(targetColor.blue) / PF_MAX_CHAN8;
    coreParams.effector.targetColor.alpha = 1.0F;
    coreParams.effector.colorAmount = static_cast<float>(
        params[PARAM_EFFECTOR_COLOR_AMOUNT]->u.sd.value) / 100.0F;
    coreParams.step.enabled = params[PARAM_STEP_ENABLE]->u.bd.value != 0;
    coreParams.step.reverse = params[PARAM_STEP_REVERSE]->u.bd.value != 0;
    coreParams.step.progress = static_cast<float>(params[PARAM_STEP_PROGRESS]->u.sd.value) / 100.0F;
    coreParams.step.falloff = static_cast<float>(params[PARAM_STEP_FALLOFF]->u.sd.value) / 100.0F;
    coreParams.step.positionX = static_cast<float>(params[PARAM_STEP_POSITION_X]->u.fs_d.value) * scaleX;
    coreParams.step.positionY = static_cast<float>(params[PARAM_STEP_POSITION_Y]->u.fs_d.value) * scaleY;
    coreParams.step.scaleAmount = static_cast<float>(params[PARAM_STEP_SCALE]->u.fs_d.value) / 100.0F;
    coreParams.step.rotationAmount = static_cast<float>(FIX_2_FLOAT(params[PARAM_STEP_ROTATION]->u.ad.value));
    coreParams.step.targetOpacity = static_cast<float>(params[PARAM_STEP_OPACITY]->u.sd.value) / 100.0F;
    coreParams.wiggle.enabled = params[PARAM_WIGGLE_ENABLE]->u.bd.value != 0;
    coreParams.wiggle.continuous = params[PARAM_WIGGLE_CONTINUOUS]->u.bd.value != 0;
    coreParams.wiggle.speed = static_cast<float>(params[PARAM_WIGGLE_SPEED]->u.fs_d.value);
    coreParams.wiggle.positionX = static_cast<float>(params[PARAM_WIGGLE_POSITION_X]->u.fs_d.value) * scaleX;
    coreParams.wiggle.positionY = static_cast<float>(params[PARAM_WIGGLE_POSITION_Y]->u.fs_d.value) * scaleY;
    coreParams.wiggle.scaleAmount = static_cast<float>(params[PARAM_WIGGLE_SCALE]->u.sd.value) / 100.0F;
    coreParams.wiggle.rotationAmount = static_cast<float>(params[PARAM_WIGGLE_ROTATION]->u.fs_d.value);
    coreParams.wiggle.opacityAmount = static_cast<float>(params[PARAM_WIGGLE_OPACITY]->u.sd.value) / 100.0F;

    coreParams.connections.enabled = params[PARAM_CONNECTIONS_ENABLE]->u.bd.value != 0;
    coreParams.connections.mode = static_cast<mo::ConnectionMode>(
        std::clamp(params[PARAM_CONNECTIONS_MODE]->u.pd.value - 1, 0, 2));
    coreParams.connections.connectionsPerClone = params[PARAM_CONNECTIONS_PER_CLONE]->u.sd.value;
    coreParams.connections.maxDistance = static_cast<float>(
        params[PARAM_CONNECTIONS_MAX_DISTANCE]->u.fs_d.value) * scaleUniform;
    coreParams.connections.closeLoop = params[PARAM_CONNECTIONS_CLOSE_LOOP]->u.bd.value != 0;
    coreParams.connections.thickness = static_cast<float>(
        params[PARAM_CONNECTIONS_THICKNESS]->u.fs_d.value) * scaleUniform;
    const auto& connectionColor = params[PARAM_CONNECTIONS_COLOR]->u.cd.value;
    coreParams.connections.color.red = static_cast<float>(connectionColor.red) / PF_MAX_CHAN8;
    coreParams.connections.color.green = static_cast<float>(connectionColor.green) / PF_MAX_CHAN8;
    coreParams.connections.color.blue = static_cast<float>(connectionColor.blue) / PF_MAX_CHAN8;
    coreParams.connections.color.alpha = 1.0F;
    coreParams.connections.opacity = static_cast<float>(
        params[PARAM_CONNECTIONS_OPACITY]->u.sd.value) / 100.0F;

    coreParams.lineEffector.enabled = params[PARAM_LINE_EFFECTOR_ENABLE]->u.bd.value != 0;
    coreParams.lineEffector.invert = params[PARAM_LINE_EFFECTOR_INVERT]->u.bd.value != 0;
    coreParams.lineEffector.shape = static_cast<mo::EffectorShape>(
        std::clamp(params[PARAM_LINE_EFFECTOR_SHAPE]->u.pd.value - 1, 0, 3));
    coreParams.lineEffector.falloff = static_cast<mo::FalloffType>(
        std::clamp(params[PARAM_LINE_EFFECTOR_FALLOFF]->u.pd.value - 1, 0, 5));
    coreParams.lineEffector.centerX = static_cast<float>(
        FIX_2_FLOAT(params[PARAM_LINE_EFFECTOR_CENTER]->u.td.x_value)) * scaleX;
    coreParams.lineEffector.centerY = static_cast<float>(
        FIX_2_FLOAT(params[PARAM_LINE_EFFECTOR_CENTER]->u.td.y_value)) * scaleY;
    coreParams.lineEffector.radius = static_cast<float>(
        params[PARAM_LINE_EFFECTOR_RADIUS]->u.fs_d.value) * scaleUniform;
    coreParams.lineEffector.innerRadius = static_cast<float>(
        params[PARAM_LINE_EFFECTOR_INNER_RADIUS]->u.fs_d.value) * scaleUniform;
    coreParams.lineEffector.power = static_cast<float>(
        params[PARAM_LINE_EFFECTOR_POWER]->u.fs_d.value);
    coreParams.lineEffector.strength = static_cast<float>(
        params[PARAM_LINE_EFFECTOR_STRENGTH]->u.sd.value) / 100.0F;
    coreParams.lineEffector.targetOpacity = static_cast<float>(
        params[PARAM_LINE_EFFECTOR_OPACITY]->u.sd.value) / 100.0F;
    coreParams.lineEffector.scaleAmount = static_cast<float>(
        params[PARAM_LINE_EFFECTOR_WIDTH]->u.fs_d.value) / 100.0F;

    coreParams.effector2.enabled = params[PARAM_EFFECTOR_2_ENABLE]->u.bd.value != 0;
    coreParams.effector2.invert = params[PARAM_EFFECTOR_2_INVERT]->u.bd.value != 0;
    coreParams.effector2.shape = static_cast<mo::EffectorShape>(
        std::clamp(params[PARAM_EFFECTOR_2_SHAPE]->u.pd.value - 1, 0, 3));
    coreParams.effector2.falloff = static_cast<mo::FalloffType>(
        std::clamp(params[PARAM_EFFECTOR_2_FALLOFF]->u.pd.value - 1, 0, 5));
    coreParams.effector2.centerX = static_cast<float>(
        FIX_2_FLOAT(params[PARAM_EFFECTOR_2_CENTER]->u.td.x_value)) * scaleX;
    coreParams.effector2.centerY = static_cast<float>(
        FIX_2_FLOAT(params[PARAM_EFFECTOR_2_CENTER]->u.td.y_value)) * scaleY;
    coreParams.effector2.radius = static_cast<float>(params[PARAM_EFFECTOR_2_RADIUS]->u.fs_d.value) * scaleUniform;
    coreParams.effector2.innerRadius = static_cast<float>(
        params[PARAM_EFFECTOR_2_INNER_RADIUS]->u.fs_d.value) * scaleUniform;
    coreParams.effector2.power = static_cast<float>(params[PARAM_EFFECTOR_2_POWER]->u.fs_d.value);
    coreParams.effector2.strength = static_cast<float>(params[PARAM_EFFECTOR_2_STRENGTH]->u.sd.value) / 100.0F;
    coreParams.effector2.positionX = static_cast<float>(params[PARAM_EFFECTOR_2_POSITION_X]->u.fs_d.value) * scaleX;
    coreParams.effector2.positionY = static_cast<float>(params[PARAM_EFFECTOR_2_POSITION_Y]->u.fs_d.value) * scaleY;
    coreParams.effector2.scaleAmount = static_cast<float>(params[PARAM_EFFECTOR_2_SCALE]->u.fs_d.value) / 100.0F;
    coreParams.effector2.rotationAmount = static_cast<float>(
        FIX_2_FLOAT(params[PARAM_EFFECTOR_2_ROTATION]->u.ad.value));
    coreParams.effector2.targetOpacity = static_cast<float>(
        params[PARAM_EFFECTOR_2_OPACITY]->u.sd.value) / 100.0F;
    coreParams.effector2.colorEnabled = params[PARAM_EFFECTOR_2_COLOR_ENABLE]->u.bd.value != 0;
    const auto& targetColor2 = params[PARAM_EFFECTOR_2_TARGET_COLOR]->u.cd.value;
    coreParams.effector2.targetColor.red = static_cast<float>(targetColor2.red) / PF_MAX_CHAN8;
    coreParams.effector2.targetColor.green = static_cast<float>(targetColor2.green) / PF_MAX_CHAN8;
    coreParams.effector2.targetColor.blue = static_cast<float>(targetColor2.blue) / PF_MAX_CHAN8;
    coreParams.effector2.targetColor.alpha = 1.0F;
    coreParams.effector2.colorAmount = static_cast<float>(
        params[PARAM_EFFECTOR_2_COLOR_AMOUNT]->u.sd.value) / 100.0F;

    return coreParams;
}

template <typename Pixel, typename Component>
mo::PixelBuffer makePixelBuffer(PF_EffectWorld* output,
                                const mo::ComponentType type,
                                const float maximum,
                                const int originX,
                                const int originY) {
    return mo::PixelBuffer{output->data,
                           output->width,
                           output->height,
                           output->rowbytes,
                           originX,
                           originY,
                           type,
                           static_cast<int>(offsetof(Pixel, alpha) / sizeof(Component)),
                           static_cast<int>(offsetof(Pixel, red) / sizeof(Component)),
                           static_cast<int>(offsetof(Pixel, green) / sizeof(Component)),
                           static_cast<int>(offsetof(Pixel, blue) / sizeof(Component)),
                           maximum};
}

PF_Err renderToWorld(PF_InData* inData,
                     PF_OutData* outData,
                     PF_EffectWorld* output,
                     const std::vector<PF_EffectWorld*>& sourceWorlds,
                     const mo::RenderParams& coreParams,
                     const bool useSource,
                     const bool legacy8Bit) {
    if (!output || !output->data || output->width <= 0 || output->height <= 0) {
        return PF_Err_BAD_CALLBACK_PARAM;
    }

    PF_PixelFormat format = PF_PixelFormat_ARGB32;
    const PF_WorldSuite2* worldSuite = nullptr;
    PF_Err err = PF_Err_NONE;
    if (!legacy8Bit) {
        err = inData->pica_basicP->AcquireSuite(kPFWorldSuite,
                                                kPFWorldSuiteVersion2,
                                                reinterpret_cast<const void**>(&worldSuite));
        if (!err && worldSuite) {
            err = worldSuite->PF_GetPixelFormat(output, &format);
        }
    }

    if (!err) {
        mo::PixelBuffer image{};
        switch (format) {
            case PF_PixelFormat_ARGB128:
                image = makePixelBuffer<PF_PixelFloat, PF_FpShort>(output,
                                                                   mo::ComponentType::Float32,
                                                                   1.0F,
                                                                   inData->output_origin_x,
                                                                   inData->output_origin_y);
                break;
            case PF_PixelFormat_ARGB64:
                image = makePixelBuffer<PF_Pixel16, A_u_short>(output,
                                                               mo::ComponentType::UInt16,
                                                               static_cast<float>(PF_MAX_CHAN16),
                                                               inData->output_origin_x,
                                                               inData->output_origin_y);
                break;
            case PF_PixelFormat_ARGB32:
                image = makePixelBuffer<PF_Pixel8, A_u_char>(output,
                                                             mo::ComponentType::UInt8,
                                                             static_cast<float>(PF_MAX_CHAN8),
                                                             inData->output_origin_x,
                                                             inData->output_origin_y);
                break;
            default:
                err = PF_Err_BAD_CALLBACK_PARAM;
                break;
        }
        if (!err) {
            mo::clear(image);
            if (useSource) {
                std::vector<mo::PixelBuffer> sourceImages;
                sourceImages.reserve(sourceWorlds.size());
                for (PF_EffectWorld* source : sourceWorlds) {
                    if (!source || !source->data) {
                        continue;
                    }
                    switch (format) {
                        case PF_PixelFormat_ARGB128:
                            sourceImages.push_back(makePixelBuffer<PF_PixelFloat, PF_FpShort>(
                                source, mo::ComponentType::Float32, 1.0F, 0, 0));
                            break;
                        case PF_PixelFormat_ARGB64:
                            sourceImages.push_back(makePixelBuffer<PF_Pixel16, A_u_short>(
                                source, mo::ComponentType::UInt16, static_cast<float>(PF_MAX_CHAN16), 0, 0));
                            break;
                        case PF_PixelFormat_ARGB32:
                        default:
                            sourceImages.push_back(makePixelBuffer<PF_Pixel8, A_u_char>(
                                source, mo::ComponentType::UInt8, static_cast<float>(PF_MAX_CHAN8), 0, 0));
                            break;
                    }
                }
                mo::renderSources(image, sourceImages, coreParams);
            } else if (!useSource) {
                mo::render(image, coreParams);
            }
        }
    }

    if (worldSuite) {
        const PF_Err releaseErr = inData->pica_basicP->ReleaseSuite(kPFWorldSuite, kPFWorldSuiteVersion2);
        if (!err) {
            err = releaseErr;
        }
    }
    (void)outData;
    return err;
}

PF_Err render(PF_InData* inData, PF_OutData* outData, PF_ParamDef* params[], PF_LayerDef* output) {
    const bool useSource = params[PARAM_SOURCE_MODE]->u.pd.value == 2;
    std::vector<PF_EffectWorld*> sources;
    if (useSource) {
        const std::array<int, 4> sourceParams{
            PARAM_SOURCE_LAYER, PARAM_SOURCE_LAYER_2, PARAM_SOURCE_LAYER_3, PARAM_SOURCE_LAYER_4};
        for (const int index : sourceParams) {
            if (params[index]->u.ld.data) {
                sources.push_back(&params[index]->u.ld);
            }
        }
    }
    return renderToWorld(inData, outData, output, sources, makeCoreParams(inData, params), useSource, true);
}

constexpr std::array<int, 121> kRenderParamIndices{
    PARAM_PRIMITIVE, PARAM_POLYGON_SIDES, PARAM_FORMATION, PARAM_CLONE_COUNT,
    PARAM_ROWS, PARAM_COLUMNS, PARAM_CENTER, PARAM_SPACING_X, PARAM_SPACING_Y,
    PARAM_RADIUS, PARAM_EFFECTOR_ENABLE, PARAM_EFFECTOR_CENTER, PARAM_EFFECTOR_SHAPE,
    PARAM_EFFECTOR_RADIUS, PARAM_EFFECTOR_INNER_RADIUS, PARAM_EFFECTOR_FALLOFF,
    PARAM_EFFECTOR_POWER, PARAM_EFFECTOR_INVERT, PARAM_EFFECTOR_STRENGTH,
    PARAM_EFFECTOR_POSITION_X, PARAM_EFFECTOR_POSITION_Y, PARAM_EFFECTOR_SCALE,
    PARAM_EFFECTOR_ROTATION, PARAM_EFFECTOR_OPACITY, PARAM_CLONE_SIZE, PARAM_ROTATION,
    PARAM_COLOR, PARAM_GLOBE_ROTATION_X, PARAM_GLOBE_ROTATION_Y, PARAM_GLOBE_ROTATION_Z,
    PARAM_GLOBE_SPEED_X, PARAM_GLOBE_SPEED_Y, PARAM_GLOBE_DEPTH_SCALE, PARAM_SOURCE_MODE,
    PARAM_SOURCE_SELECTION, PARAM_SOURCE_RANDOM_SEED, PARAM_PATH_START, PARAM_PATH_CONTROL_1,
    PARAM_PATH_CONTROL_2, PARAM_PATH_END, PARAM_PATH_ALIGN, PARAM_PATH_ROTATION_OFFSET,
    PARAM_PATH_POSITION_OFFSET,
    PARAM_VARIATION_SEED, PARAM_POSITION_JITTER_X, PARAM_POSITION_JITTER_Y,
    PARAM_SCALE_JITTER, PARAM_ROTATION_JITTER, PARAM_OPACITY_JITTER,
    PARAM_PALETTE_ENABLE, PARAM_PALETTE_SIZE, PARAM_PALETTE_COLOR_2,
    PARAM_PALETTE_COLOR_3, PARAM_PALETTE_COLOR_4, PARAM_PALETTE_SELECTION,
    PARAM_PALETTE_SEED, PARAM_PALETTE_TINT_SOURCES, PARAM_EFFECTOR_COLOR_ENABLE,
    PARAM_EFFECTOR_TARGET_COLOR, PARAM_EFFECTOR_COLOR_AMOUNT, PARAM_STEP_ENABLE,
    PARAM_STEP_PROGRESS, PARAM_STEP_FALLOFF, PARAM_STEP_REVERSE, PARAM_STEP_POSITION_X,
    PARAM_STEP_POSITION_Y, PARAM_STEP_SCALE, PARAM_STEP_ROTATION, PARAM_STEP_OPACITY,
    PARAM_FORMATION_ROTATION, PARAM_CIRCLE_RINGS, PARAM_Z_CIRCLE_TILT, PARAM_Z_CIRCLE_ROTATION,
    PARAM_Z_CIRCLE_SPEED, PARAM_Z_CIRCLE_DEPTH, PARAM_RANDOM_SIZE,
    PARAM_VARIATION_SPEED, PARAM_WIGGLE_ENABLE, PARAM_WIGGLE_CONTINUOUS, PARAM_WIGGLE_SPEED,
    PARAM_WIGGLE_POSITION_X, PARAM_WIGGLE_POSITION_Y, PARAM_WIGGLE_SCALE,
    PARAM_WIGGLE_ROTATION, PARAM_WIGGLE_OPACITY, PARAM_EFFECTOR_2_ENABLE,
    PARAM_EFFECTOR_2_CENTER, PARAM_EFFECTOR_2_SHAPE, PARAM_EFFECTOR_2_RADIUS,
    PARAM_EFFECTOR_2_INNER_RADIUS, PARAM_EFFECTOR_2_FALLOFF, PARAM_EFFECTOR_2_POWER,
    PARAM_EFFECTOR_2_INVERT, PARAM_EFFECTOR_2_STRENGTH, PARAM_EFFECTOR_2_POSITION_X,
    PARAM_EFFECTOR_2_POSITION_Y, PARAM_EFFECTOR_2_SCALE, PARAM_EFFECTOR_2_ROTATION,
    PARAM_EFFECTOR_2_OPACITY, PARAM_EFFECTOR_2_COLOR_ENABLE,
    PARAM_EFFECTOR_2_TARGET_COLOR, PARAM_EFFECTOR_2_COLOR_AMOUNT,
    PARAM_CONNECTIONS_ENABLE, PARAM_CONNECTIONS_MODE, PARAM_CONNECTIONS_PER_CLONE,
    PARAM_CONNECTIONS_MAX_DISTANCE, PARAM_CONNECTIONS_CLOSE_LOOP,
    PARAM_CONNECTIONS_THICKNESS, PARAM_CONNECTIONS_COLOR, PARAM_CONNECTIONS_OPACITY,
    PARAM_LINE_EFFECTOR_ENABLE, PARAM_LINE_EFFECTOR_CENTER, PARAM_LINE_EFFECTOR_SHAPE,
    PARAM_LINE_EFFECTOR_RADIUS, PARAM_LINE_EFFECTOR_INNER_RADIUS,
    PARAM_LINE_EFFECTOR_FALLOFF, PARAM_LINE_EFFECTOR_POWER, PARAM_LINE_EFFECTOR_INVERT,
    PARAM_LINE_EFFECTOR_STRENGTH, PARAM_LINE_EFFECTOR_OPACITY, PARAM_LINE_EFFECTOR_WIDTH};

PF_Err checkoutCoreParams(PF_InData* inData, mo::RenderParams& result, bool& useSource) {
    std::array<PF_ParamDef, PARAM_COUNT> definitions{};
    std::array<PF_ParamDef*, PARAM_COUNT> pointers{};
    PF_Err err = PF_Err_NONE;
    std::size_t checked = 0;
    for (; checked < kRenderParamIndices.size() && !err; ++checked) {
        const int index = kRenderParamIndices[checked];
        AEFX_CLR_STRUCT(definitions[index]);
        err = PF_CHECKOUT_PARAM(inData,
                                index,
                                inData->current_time,
                                inData->time_step,
                                inData->time_scale,
                                &definitions[index]);
        if (!err) {
            pointers[index] = &definitions[index];
        }
    }
    // Opacity is intentionally appended here to keep the compile-time list readable.
    if (!err) {
        AEFX_CLR_STRUCT(definitions[PARAM_OPACITY]);
        err = PF_CHECKOUT_PARAM(inData,
                                PARAM_OPACITY,
                                inData->current_time,
                                inData->time_step,
                                inData->time_scale,
                                &definitions[PARAM_OPACITY]);
        pointers[PARAM_OPACITY] = &definitions[PARAM_OPACITY];
    }
    if (!err) {
        result = makeCoreParams(inData, pointers.data());
        useSource = definitions[PARAM_SOURCE_MODE].u.pd.value == 2;
    }
    if (pointers[PARAM_OPACITY]) {
        const PF_Err checkinErr = PF_CHECKIN_PARAM(inData, &definitions[PARAM_OPACITY]);
        if (!err) {
            err = checkinErr;
        }
    }
    while (checked > 0) {
        --checked;
        const int index = kRenderParamIndices[checked];
        if (pointers[index]) {
            const PF_Err checkinErr = PF_CHECKIN_PARAM(inData, &definitions[index]);
            if (!err) {
                err = checkinErr;
            }
        }
    }
    return err;
}

struct PreRenderData {
    mo::RenderParams params{};
    bool useSource{};
    std::array<bool, 4> sourceChecked{};
};

constexpr std::array<int, 4> kSourceLayerParams{
    PARAM_SOURCE_LAYER, PARAM_SOURCE_LAYER_2, PARAM_SOURCE_LAYER_3, PARAM_SOURCE_LAYER_4};

void deletePreRenderData(void* data) {
    delete static_cast<PreRenderData*>(data);
}

PF_Err preRender(PF_InData* inData, PF_PreRenderExtra* extra) {
    if (!extra || !extra->input || !extra->output) {
        return PF_Err_BAD_CALLBACK_PARAM;
    }
    auto* snapshot = new (std::nothrow) PreRenderData{};
    if (!snapshot) {
        return PF_Err_OUT_OF_MEMORY;
    }
    PF_Err err = checkoutCoreParams(inData, snapshot->params, snapshot->useSource);
    if (err) {
        delete snapshot;
        return err;
    }
    PF_RenderRequest request = extra->input->output_request;
    PF_CheckoutResult inputResult{};
    err = extra->cb->checkout_layer(inData->effect_ref,
                                    PARAM_INPUT,
                                    PARAM_INPUT,
                                    &request,
                                    inData->current_time,
                                    inData->time_step,
                                    inData->time_scale,
                                    &inputResult);
    if (!err && snapshot->useSource) {
        for (std::size_t slot = 0; slot < kSourceLayerParams.size() && !err; ++slot) {
            const int parameter = kSourceLayerParams[slot];
            PF_RenderRequest sourceRequest = request;
            sourceRequest.rect = inputResult.max_result_rect;
            PF_CheckoutResult sourceResult{};
            err = extra->cb->checkout_layer(inData->effect_ref,
                                            parameter,
                                            parameter,
                                            &sourceRequest,
                                            inData->current_time,
                                            inData->time_step,
                                            inData->time_scale,
                                            &sourceResult);
            if (!err) {
                snapshot->sourceChecked[slot] = sourceResult.max_result_rect.right >
                                                    sourceResult.max_result_rect.left &&
                                                sourceResult.max_result_rect.bottom >
                                                    sourceResult.max_result_rect.top;
            }
        }
    }
    if (!err) {
        extra->output->pre_render_data = snapshot;
        extra->output->delete_pre_render_data_func = deletePreRenderData;
        extra->output->result_rect = inputResult.result_rect;
        extra->output->max_result_rect = inputResult.max_result_rect;
        extra->output->solid = false;
    } else {
        delete snapshot;
    }
    return err;
}

PF_Err smartRender(PF_InData* inData, PF_OutData* outData, PF_SmartRenderExtra* extra) {
    if (!extra || !extra->input || !extra->cb || !extra->input->pre_render_data) {
        return PF_Err_BAD_CALLBACK_PARAM;
    }
    PF_EffectWorld* output = nullptr;
    PF_EffectWorld* input = nullptr;
    std::array<PF_EffectWorld*, 4> sourceSlots{};
    std::vector<PF_EffectWorld*> sources;
    const auto* snapshot = static_cast<const PreRenderData*>(extra->input->pre_render_data);
    PF_Err err = extra->cb->checkout_layer_pixels(inData->effect_ref, PARAM_INPUT, &input);
    if (!err && snapshot->useSource) {
        for (std::size_t slot = 0; slot < kSourceLayerParams.size() && !err; ++slot) {
            if (snapshot->sourceChecked[slot]) {
                err = extra->cb->checkout_layer_pixels(inData->effect_ref,
                                                       kSourceLayerParams[slot],
                                                       &sourceSlots[slot]);
                if (!err && sourceSlots[slot]) {
                    sources.push_back(sourceSlots[slot]);
                }
            }
        }
    }
    if (!err) {
        err = extra->cb->checkout_output(inData->effect_ref, &output);
    }
    if (!err) {
        err = renderToWorld(inData,
                            outData,
                            output,
                            sources,
                            snapshot->params,
                            snapshot->useSource,
                            false);
    }
    for (std::size_t slot = kSourceLayerParams.size(); slot > 0; --slot) {
        const std::size_t index = slot - 1;
        if (sourceSlots[index]) {
            const PF_Err checkinErr = extra->cb->checkin_layer_pixels(inData->effect_ref,
                                                                      kSourceLayerParams[index]);
            if (!err) {
                err = checkinErr;
            }
        }
    }
    if (input) {
        const PF_Err checkinErr = extra->cb->checkin_layer_pixels(inData->effect_ref, PARAM_INPUT);
        if (!err) {
            err = checkinErr;
        }
    }
    return err;
}

PF_Err drawSingleEffectorGuide(PF_InData* inData,
                               PF_OutData* outData,
                               PF_ParamDef* params[],
                               PF_EventExtra* eventExtra,
                               const int enableParam,
                               const int centerParam,
                               const int shapeParam,
                               const int radiusParam,
                               const int innerRadiusParam) {
    if (!eventExtra || eventExtra->e_type != PF_Event_DRAW || !eventExtra->contextH ||
        !params[enableParam]->u.bd.value) {
        return PF_Err_NONE;
    }
    const PF_WindowType windowType = (*eventExtra->contextH)->w_type;
    if (windowType != PF_Window_COMP && windowType != PF_Window_LAYER) {
        return PF_Err_NONE;
    }

    PF_Err err = PF_Err_NONE;
    DRAWBOT_Suites drawbot{};
    DRAWBOT_DrawRef drawingRef = nullptr;
    AEGP_SuiteHandler suites(inData->pica_basicP);
    err = AEFX_AcquireDrawbotSuites(inData, outData, &drawbot);
    if (!err) {
        err = suites.EffectCustomUISuite1()->PF_GetDrawingReference(eventExtra->contextH, &drawingRef);
    }
    if (err || !drawingRef) {
        AEFX_ReleaseDrawbotSuites(inData, outData);
        return err;
    }

    DRAWBOT_SupplierRef supplierRef = nullptr;
    err = drawbot.drawbot_suiteP->GetSupplier(drawingRef, &supplierRef);
    if (!err) {
        DRAWBOT_PathP path(drawbot.supplier_suiteP, supplierRef);
        const float centerX = static_cast<float>(FIX_2_FLOAT(params[centerParam]->u.td.x_value));
        const float centerY = static_cast<float>(FIX_2_FLOAT(params[centerParam]->u.td.y_value));
        const float outer = std::max(1.0F, static_cast<float>(params[radiusParam]->u.fs_d.value));
        const float inner = std::clamp(static_cast<float>(params[innerRadiusParam]->u.fs_d.value), 0.0F, outer);
        const auto shape = std::clamp(params[shapeParam]->u.pd.value - 1, 0, 3);

        auto addPoint = [&](const float sourceX, const float sourceY, const bool move) -> PF_Err {
            PF_FixedPoint point{FLOAT2FIX(sourceX), FLOAT2FIX(sourceY)};
            if (windowType == PF_Window_COMP) {
                eventExtra->cbs.layer_to_comp(eventExtra->cbs.refcon,
                                              eventExtra->contextH,
                                              inData->current_time,
                                              inData->time_scale,
                                              &point);
            }
            eventExtra->cbs.source_to_frame(eventExtra->cbs.refcon, eventExtra->contextH, &point);
            if (move) {
                return drawbot.path_suiteP->MoveTo(path,
                                                   static_cast<float>(FIX_2_FLOAT(point.x)),
                                                   static_cast<float>(FIX_2_FLOAT(point.y)));
            }
            return drawbot.path_suiteP->LineTo(path,
                                               static_cast<float>(FIX_2_FLOAT(point.x)),
                                               static_cast<float>(FIX_2_FLOAT(point.y)));
        };

        auto addGuide = [&](const float radius) -> PF_Err {
            if (radius <= 0.0F) {
                return PF_Err_NONE;
            }
            PF_Err guideErr = PF_Err_NONE;
            if (shape == static_cast<int>(mo::EffectorShape::Circle)) {
                constexpr int points = 64;
                for (int index = 0; index <= points && !guideErr; ++index) {
                    const float angle = 2.0F * 3.14159265358979323846F * static_cast<float>(index) /
                                        static_cast<float>(points);
                    guideErr = addPoint(centerX + std::cos(angle) * radius,
                                        centerY + std::sin(angle) * radius,
                                        index == 0);
                }
            } else if (shape == static_cast<int>(mo::EffectorShape::Box)) {
                const std::array<std::array<float, 2>, 5> points{{
                    {{centerX - radius, centerY - radius}}, {{centerX + radius, centerY - radius}},
                    {{centerX + radius, centerY + radius}}, {{centerX - radius, centerY + radius}},
                    {{centerX - radius, centerY - radius}}}};
                for (std::size_t index = 0; index < points.size() && !guideErr; ++index) {
                    guideErr = addPoint(points[index][0], points[index][1], index == 0);
                }
            } else if (shape == static_cast<int>(mo::EffectorShape::LinearX)) {
                guideErr = addPoint(centerX - radius, centerY, true);
                if (!guideErr) guideErr = addPoint(centerX + radius, centerY, false);
            } else {
                guideErr = addPoint(centerX, centerY - radius, true);
                if (!guideErr) guideErr = addPoint(centerX, centerY + radius, false);
            }
            return guideErr;
        };

        err = addGuide(outer);
        if (!err) {
            err = addGuide(inner);
        }
        if (!err && inData->appl_id != kAppID_Premiere) {
            err = suites.EffectCustomUIOverlayThemeSuite1()->PF_StrokePath(drawingRef, path, FALSE);
        }
    }
    AEFX_ReleaseDrawbotSuites(inData, outData);
    if (!err) {
        eventExtra->evt_out_flags = PF_EO_HANDLED_EVENT;
    }
    return err;
}

PF_Err drawEffectorGuides(PF_InData* inData,
                          PF_OutData* outData,
                          PF_ParamDef* params[],
                          PF_EventExtra* eventExtra) {
    PF_Err err = drawSingleEffectorGuide(inData, outData, params, eventExtra,
                                         PARAM_EFFECTOR_ENABLE, PARAM_EFFECTOR_CENTER,
                                         PARAM_EFFECTOR_SHAPE, PARAM_EFFECTOR_RADIUS,
                                         PARAM_EFFECTOR_INNER_RADIUS);
    const PF_Err secondErr = drawSingleEffectorGuide(inData, outData, params, eventExtra,
                                                     PARAM_EFFECTOR_2_ENABLE,
                                                     PARAM_EFFECTOR_2_CENTER,
                                                     PARAM_EFFECTOR_2_SHAPE,
                                                     PARAM_EFFECTOR_2_RADIUS,
                                                     PARAM_EFFECTOR_2_INNER_RADIUS);
    const PF_Err lineErr = drawSingleEffectorGuide(inData, outData, params, eventExtra,
                                                   PARAM_LINE_EFFECTOR_ENABLE,
                                                   PARAM_LINE_EFFECTOR_CENTER,
                                                   PARAM_LINE_EFFECTOR_SHAPE,
                                                   PARAM_LINE_EFFECTOR_RADIUS,
                                                   PARAM_LINE_EFFECTOR_INNER_RADIUS);
    return err ? err : (secondErr ? secondErr : lineErr);
}

PF_Err drawPathGuide(PF_InData* inData,
                     PF_OutData* outData,
                     PF_ParamDef* params[],
                     PF_EventExtra* eventExtra) {
    if (!eventExtra || eventExtra->e_type != PF_Event_DRAW || !eventExtra->contextH ||
        params[PARAM_FORMATION]->u.pd.value != 6) {
        return PF_Err_NONE;
    }
    const PF_WindowType windowType = (*eventExtra->contextH)->w_type;
    if (windowType != PF_Window_COMP && windowType != PF_Window_LAYER) {
        return PF_Err_NONE;
    }

    PF_Err err = PF_Err_NONE;
    DRAWBOT_Suites drawbot{};
    DRAWBOT_DrawRef drawingRef = nullptr;
    AEGP_SuiteHandler suites(inData->pica_basicP);
    err = AEFX_AcquireDrawbotSuites(inData, outData, &drawbot);
    if (!err) {
        err = suites.EffectCustomUISuite1()->PF_GetDrawingReference(eventExtra->contextH, &drawingRef);
    }
    if (err || !drawingRef) {
        AEFX_ReleaseDrawbotSuites(inData, outData);
        return err;
    }

    DRAWBOT_SupplierRef supplierRef = nullptr;
    err = drawbot.drawbot_suiteP->GetSupplier(drawingRef, &supplierRef);
    if (!err) {
        DRAWBOT_PathP path(drawbot.supplier_suiteP, supplierRef);
        const std::array<std::array<float, 2>, 4> controls{{
            {{static_cast<float>(FIX_2_FLOAT(params[PARAM_PATH_START]->u.td.x_value)),
              static_cast<float>(FIX_2_FLOAT(params[PARAM_PATH_START]->u.td.y_value))}},
            {{static_cast<float>(FIX_2_FLOAT(params[PARAM_PATH_CONTROL_1]->u.td.x_value)),
              static_cast<float>(FIX_2_FLOAT(params[PARAM_PATH_CONTROL_1]->u.td.y_value))}},
            {{static_cast<float>(FIX_2_FLOAT(params[PARAM_PATH_CONTROL_2]->u.td.x_value)),
              static_cast<float>(FIX_2_FLOAT(params[PARAM_PATH_CONTROL_2]->u.td.y_value))}},
            {{static_cast<float>(FIX_2_FLOAT(params[PARAM_PATH_END]->u.td.x_value)),
              static_cast<float>(FIX_2_FLOAT(params[PARAM_PATH_END]->u.td.y_value))}}}};

        auto addPoint = [&](const float sourceX, const float sourceY, const bool move) -> PF_Err {
            PF_FixedPoint point{FLOAT2FIX(sourceX), FLOAT2FIX(sourceY)};
            if (windowType == PF_Window_COMP) {
                eventExtra->cbs.layer_to_comp(eventExtra->cbs.refcon,
                                              eventExtra->contextH,
                                              inData->current_time,
                                              inData->time_scale,
                                              &point);
            }
            eventExtra->cbs.source_to_frame(eventExtra->cbs.refcon, eventExtra->contextH, &point);
            const float x = static_cast<float>(FIX_2_FLOAT(point.x));
            const float y = static_cast<float>(FIX_2_FLOAT(point.y));
            return move ? drawbot.path_suiteP->MoveTo(path, x, y) :
                          drawbot.path_suiteP->LineTo(path, x, y);
        };

        for (std::size_t index = 0; index < controls.size() && !err; ++index) {
            err = addPoint(controls[index][0], controls[index][1], index == 0);
        }
        constexpr int samples = 64;
        for (int sample = 0; sample <= samples && !err; ++sample) {
            const float t = static_cast<float>(sample) / static_cast<float>(samples);
            const float inverse = 1.0F - t;
            const float a = inverse * inverse * inverse;
            const float b = 3.0F * inverse * inverse * t;
            const float c = 3.0F * inverse * t * t;
            const float d = t * t * t;
            const float x = a * controls[0][0] + b * controls[1][0] +
                            c * controls[2][0] + d * controls[3][0];
            const float y = a * controls[0][1] + b * controls[1][1] +
                            c * controls[2][1] + d * controls[3][1];
            err = addPoint(x, y, sample == 0);
        }
        if (!err && inData->appl_id != kAppID_Premiere) {
            err = suites.EffectCustomUIOverlayThemeSuite1()->PF_StrokePath(drawingRef, path, FALSE);
        }
    }
    AEFX_ReleaseDrawbotSuites(inData, outData);
    if (!err) {
        eventExtra->evt_out_flags = PF_EO_HANDLED_EVENT;
    }
    return err;
}

PF_Err drawGuides(PF_InData* inData,
                  PF_OutData* outData,
                  PF_ParamDef* params[],
                  PF_EventExtra* eventExtra) {
    PF_Err err = drawEffectorGuides(inData, outData, params, eventExtra);
    const PF_Err pathErr = drawPathGuide(inData, outData, params, eventExtra);
    return err ? err : pathErr;
}

}  // namespace

extern "C" DllExport PF_Err PluginDataEntryFunction2(PF_PluginDataPtr inPtr,
                                                       PF_PluginDataCB2 callback,
                                                       SPBasicSuite* basicSuite,
                                                       const char* hostName,
                                                       const char* hostVersion) {
    (void)basicSuite;
    (void)hostName;
    (void)hostVersion;
    PF_Err result = PF_Err_INVALID_CALLBACK;
    PF_REGISTER_EFFECT_EXT2(inPtr,
                            callback,
                            "MO Effector Native",
                            "com.brunojorri.MOEffectorNative",
                            "MO Tools",
                            AE_RESERVED_INFO,
                            "EffectMain",
                            "https://www.instagram.com/brunojorri_work/");
    return result;
}

PF_Err EffectMain(PF_Cmd cmd,
                  PF_InData* inData,
                  PF_OutData* outData,
                  PF_ParamDef* params[],
                  PF_LayerDef* output,
                  void* extra) {
    try {
        switch (cmd) {
            case PF_Cmd_ABOUT:
                return about(inData, outData);
            case PF_Cmd_GLOBAL_SETUP:
                return globalSetup(outData);
            case PF_Cmd_PARAMS_SETUP:
                return paramsSetup(inData, outData);
            case PF_Cmd_RENDER:
                return render(inData, outData, params, output);
            case PF_Cmd_SMART_PRE_RENDER:
                return preRender(inData, static_cast<PF_PreRenderExtra*>(extra));
            case PF_Cmd_SMART_RENDER:
                return smartRender(inData, outData, static_cast<PF_SmartRenderExtra*>(extra));
            case PF_Cmd_EVENT:
                return drawGuides(inData, outData, params, static_cast<PF_EventExtra*>(extra));
            default:
                return PF_Err_NONE;
        }
    } catch (...) {
        return PF_Err_INTERNAL_STRUCT_DAMAGED;
    }
}

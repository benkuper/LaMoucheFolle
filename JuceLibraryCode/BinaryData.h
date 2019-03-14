/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   badcalib_png;
    const int            badcalib_pngSize = 9542;

    extern const char*   battery_problem_png;
    const int            battery_problem_pngSize = 6427;

    extern const char*   calibrating_png;
    const int            calibrating_pngSize = 12697;

    extern const char*   charging_png;
    const int            charging_pngSize = 5831;

    extern const char*   config_problem_png;
    const int            config_problem_pngSize = 13847;

    extern const char*   connect_bt_png;
    const int            connect_bt_pngSize = 9077;

    extern const char*   default_cflayout;
    const int            default_cflayoutSize = 1938;

    extern const char*   disconnect_bt_png;
    const int            disconnect_bt_pngSize = 12150;

    extern const char*   drone_badcalib_png;
    const int            drone_badcalib_pngSize = 19447;

    extern const char*   drone_connecting_png;
    const int            drone_connecting_pngSize = 15975;

    extern const char*   drone_error_png;
    const int            drone_error_pngSize = 15534;

    extern const char*   drone_lowbattery_png;
    const int            drone_lowbattery_pngSize = 15818;

    extern const char*   drone_ok_png;
    const int            drone_ok_pngSize = 15244;

    extern const char*   drone_poweroff_png;
    const int            drone_poweroff_pngSize = 16207;

    extern const char*   drone_poweron_png;
    const int            drone_poweron_pngSize = 16184;

    extern const char*   drone_warning_png;
    const int            drone_warning_pngSize = 14497;

    extern const char*   flying_png;
    const int            flying_pngSize = 11367;

    extern const char*   health_analysis_png;
    const int            health_analysis_pngSize = 11070;

    extern const char*   icon_png;
    const int            icon_pngSize = 66595;

    extern const char*   low_battery_png;
    const int            low_battery_pngSize = 5222;

    extern const char*   parachute_png;
    const int            parachute_pngSize = 12425;

    extern const char*   startup_png;
    const int            startup_pngSize = 12436;

    extern const char*   upsidedown_png;
    const int            upsidedown_pngSize = 20068;

    extern const char*   warning_png;
    const int            warning_pngSize = 4699;

    extern const char*   warning_red_png;
    const int            warning_red_pngSize = 3997;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 25;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}

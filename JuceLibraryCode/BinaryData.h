/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   charging_png;
    const int            charging_pngSize = 3774;

    extern const char*   default_cflayout;
    const int            default_cflayoutSize = 1593;

    extern const char*   flying_png;
    const int            flying_pngSize = 2550;

    extern const char*   icon_png;
    const int            icon_pngSize = 66595;

    extern const char*   in_png;
    const int            in_pngSize = 20744;

    extern const char*   lowbattery_png;
    const int            lowbattery_pngSize = 2544;

    extern const char*   out_png;
    const int            out_pngSize = 20427;

    extern const char*   transparent_png;
    const int            transparent_pngSize = 2311;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 8;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes) noexcept;

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8) noexcept;
}

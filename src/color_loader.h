#pragma once
#ifndef CATA_SRC_COLOR_LOADER_H
#define CATA_SRC_COLOR_LOADER_H

#include <array>
#include <fstream>
#include <map>
#include <string>

#include "debug.h"
#include "filesystem.h"
#include "json.h"
#include "path_info.h"

template<typename ColorType>
class color_loader
{
    public:
        static constexpr size_t COLOR_NAMES_COUNT = 16;

    private:
        static ColorType from_rgb( int r, int g, int b );

        std::map<std::string, ColorType> consolecolors;

        // color names as read from the json file
        static const std::array<std::string, COLOR_NAMES_COUNT> &main_color_names() {
            static const std::array<std::string, COLOR_NAMES_COUNT> names{ { "BLACK", "RED", "GREEN",
                    "BROWN", "BLUE", "MAGENTA", "CYAN", "GRAY", "DGRAY", "LRED", "LGREEN", "YELLOW",
                    "LBLUE", "LMAGENTA", "LCYAN", "WHITE"
                } };
            return names;
        }

        void load_colors( const JsonObject &jsobj );
        ColorType ccolor( const std::string &color ) const;
        void load_colorfile( const std::string &path );

    public:
        /// @throws std::exception upon any kind of error.
        void load( std::array<ColorType, COLOR_NAMES_COUNT> &windowsPalette );
};

class SDL_Color;
class RGBQUAD;

template<>
SDL_Color color_loader<SDL_Color>::from_rgb( const int r, const int g, const int b );
template<>
RGBQUAD color_loader<RGBQUAD>::from_rgb( const int r, const int g, const int b );

extern template class color_loader<SDL_Color>;
extern template class color_loader<RGBQUAD>;


#endif // CATA_SRC_COLOR_LOADER_H

#pragma once
#ifndef PANELS_H
#define PANELS_H
#include "color.h"
#include "game.h"
#include "input.h"
#include <string>


class player;
namespace catacurses
{
class window;
} // namespace catacurses
void decorate_panel( const std::string name, const catacurses::window &w );
void draw_character( player &p, const catacurses::window &w );
void draw_environment( const player &u, const catacurses::window &w );
void draw_messages( const catacurses::window &w );
void draw_modifiers( const player &u, const catacurses::window &w );
void draw_lookaround( const catacurses::window &w );
void draw_mminimap( const catacurses::window &w );
void draw_compass( const catacurses::window &w );
void draw_panel_adm( const catacurses::window &w );
void debug();
nc_color stat_color( int stat );
std::string time_approx();
std::string get_temp( const player &u );
std::string get_moon();
std::pair<nc_color, int> morale_stat( const player &u );
std::pair<nc_color, std::string> hunger_stat( const player &u );
std::pair<nc_color, std::string> temp_stat( const player &u );
std::pair<nc_color, std::string> thirst_stat( const player &u );
std::pair<nc_color, std::string> rest_stat( const player &u );
std::pair<nc_color, std::string> pain_stat( const player &u );
std::string get_armor( const player &u, body_part bp, const catacurses::window &w );
#endif

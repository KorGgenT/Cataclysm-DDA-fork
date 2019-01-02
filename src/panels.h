#pragma once
#ifndef PANELS_H
#define PANELS_H
#include <string>

class player;
namespace catacurses
{
class window;
} // namespace catacurses
void decorate_panel( const std::string name, const catacurses::window &w );
void draw_character( const player &p, const catacurses::window &w );
void draw_environment( const player &u, const catacurses::window &w );
void draw_messages( const catacurses::window &w );
void draw_lookaround( const catacurses::window &w );
void draw_mminimap( const catacurses::window &w );
void draw_compass( const catacurses::window &w );

#endif

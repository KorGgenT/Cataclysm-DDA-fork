#pragma once
#ifndef PANELS_H
#define PANELS_H


class player;
namespace catacurses
{
class window;
} // namespace catacurses

void draw_mypanel( const player &p, const catacurses::window &w );
#endif

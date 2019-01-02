#include "panels.h"

#include "cata_utility.h"
#include "color.h"
#include "cursesdef.h"
#include "game.h"
// #include "live_view.h"
#include "messages.h"
#include "overmap.h"
#include "overmap_ui.h"
#include "overmapbuffer.h"
#include "output.h"
#include "player.h"
#include "weather.h"
#include "weather_gen.h"
#include <cmath>
#include <string>

void decorate_panel( const std::string name, const catacurses::window &w )
{
    werase( w );
    draw_border( w );

    static const char *title_prefix = "< ";
    const std::string title = name;
    static const char *title_suffix = " >";
    static const std::string full_title = string_format( "%s%s%s", title_prefix, title, title_suffix );
    const int start_pos = center_text_pos( full_title.c_str(), 0, getmaxx( w ) - 1 );
    mvwprintz( w, 0, start_pos, c_white, title_prefix );
    wprintz( w, c_green, title );
    wprintz( w, c_white, title_suffix );
}

void draw_character( const player &u, const catacurses::window &w )
{
    // character panel
    static const std::string title = _( "Character" );
    decorate_panel( title, w );
    const int dy  = 1;
    static std::array<body_part, 7> part = {{
            bp_head, bp_torso, bp_arm_l, bp_arm_r, bp_leg_l, bp_leg_r, num_bp
        }
    };
    for( size_t i = 0; i < part.size(); i++ ) {
        const std::string str = ( i == part.size() - 1 ) ?
                                _( "POWER" ) : body_part_hp_bar_ui_text( part[i] );
        wmove( w, ( i + 1 ) * dy, 1 );
        wprintz( w, u.limb_color( part[i], true, true, true ), " " );
        wprintz( w, u.limb_color( part[i], true, true, true ),
                 str + " : " + std::to_string( u.hp_cur[i] ) );

    }

    wrefresh( w );
}

void draw_environment( const player &u, const catacurses::window &w )
{
    // environment panel
    const std::string title = _( "Environment" );
    decorate_panel( title, w );
    // display location
    const oter_id &cur_ter = overmap_buffer.ter( u.global_omt_location() );
    // wrefresh( s_window );
    mvwprintz( w, 1, 1, c_light_gray, "Location: " );
    wprintz( w, c_white, utf8_truncate( cur_ter->get_name(), getmaxx( w ) ) );
    // display weather
    if( g->get_levz() < 0 ) {
        mvwprintz( w, 2, 1, c_light_gray, _( "Underground" ) );
    } else {
        mvwprintz( w, 2, 1, c_light_gray, _( "Weather :" ) );
        wprintz( w, weather_data( g->weather ).color, " %s", weather_data( g->weather ).name.c_str() );
    }
    // display lighting
    const auto ll = get_light_level( g->u.fine_detail_vision_mod() );
    mvwprintz( w, 3, 1, c_light_gray, "%s ", _( "Lighting:" ) );
    wprintz( w, ll.second, ll.first.c_str() );
    // display date
    mvwprintz( w, 4, 1, c_light_gray, _( "Date    : %s, day %d" ),
               calendar::name_season( season_of_year( calendar::turn ) ),
               day_of_season<int>( calendar::turn ) + 1 );
    // display time
    if( u.has_watch() ) {
        mvwprintz( w, 5, 1, c_light_gray, _( "Time    : %s, day %d" ),
                   to_string_time_of_day( calendar::turn ) );
        // wprintz( time_window, c_white, to_string_time_of_day( calendar::turn ) );
    } else if( g->get_levz() >= 0 ) {
        // const int iHour = hour_of_day<int>( calendar::turn );
        mvwprintz( w, 5, 1, c_light_gray, _( "Time    : Around noon" ) );
    } else {
        mvwprintz( w, 5, 1, c_light_gray, _( "Time    : ???" ) );
    }
    /*
    std::vector<std::pair<char, nc_color> > vGlyphs;
    vGlyphs.push_back( std::make_pair( '_', c_red ) );
    vGlyphs.push_back( std::make_pair( '_', c_cyan ) );
    vGlyphs.push_back( std::make_pair( '.', c_brown ) );
    vGlyphs.push_back( std::make_pair( ',', c_blue ) );
    vGlyphs.push_back( std::make_pair( '+', c_yellow ) );
    vGlyphs.push_back( std::make_pair( 'c', c_light_blue ) );
    vGlyphs.push_back( std::make_pair( '*', c_yellow ) );
    vGlyphs.push_back( std::make_pair( 'C', c_white ) );
    vGlyphs.push_back( std::make_pair( '+', c_yellow ) );
    vGlyphs.push_back( std::make_pair( 'c', c_light_blue ) );
    vGlyphs.push_back( std::make_pair( '.', c_brown ) );
    vGlyphs.push_back( std::make_pair( ',', c_blue ) );
    vGlyphs.push_back( std::make_pair( '_', c_red ) );
    vGlyphs.push_back( std::make_pair( '_', c_cyan ) );

    const int iHour = hour_of_day<int>( calendar::turn );
    wprintz( w, c_white, "[" );
    bool bAddTrail = false;

    for( int i = 0; i < 14; i += 2 ) {
        if( iHour >= 8 + i && iHour <= 13 + ( i / 2 ) ) {
            wputch( w, hilite( c_white ), ' ' );

        } else if( iHour >= 6 + i && iHour <= 7 + i ) {
            wputch( w, hilite( vGlyphs[i].second ), vGlyphs[i].first );
            bAddTrail = true;

        } else if( iHour >= ( 18 + i ) % 24 && iHour <= ( 19 + i ) % 24 ) {
            wputch( w, vGlyphs[i + 1].second, vGlyphs[i + 1].first );

        } else if( bAddTrail && iHour >= 6 + ( i / 2 ) ) {
            wputch( w, hilite( c_white ), ' ' );

        } else {
            wputch( w, c_white, ' ' );
        }
    }
    wprintz( w, c_white, "]" );

    } else {
    wprintz( w, c_white, _( "Time: ???" ) );
    }
    */
    mvwprintz( w, 6, 1, c_light_gray, _( "Temp    : ???" ) );
    mvwprintz( w, 7, 1, c_light_gray, _( "Rad     : ???" ) );
    // mvwprintz( w, 8, 1, c_light_gray, _( "Moon    : ???" ) );
    wrefresh( w );
}

void draw_messages( const catacurses::window &w )
{
    // messages panel
    const std::string title = _( "Messages" );
    decorate_panel( title, w );
    // werase( w_messages );

    // Print liveview or monster info and start log messages output below it.
    // int topline = liveview.draw( w_messages, getmaxy( w_messages ) );
    // if( topline == 0 ) {
    //     topline = mon_info( w_messages ) + 2;
    // }
    int line = getmaxy( w ) - 2;
    int maxlength = getmaxx( w );
    Messages::display_messages( w, 1, 1 /*topline*/, maxlength, line );
    wrefresh( w );
}

void draw_lookaround( const catacurses::window &w )
{
    // look_around panel
    const std::string title = _( "Look around" );
    decorate_panel( title, w );
    wrefresh( w );
}

void draw_mminimap( const catacurses::window &w )
{
    // minimap panel
    const std::string title = _( "Minimap" );
    decorate_panel( title, w );
    wrefresh( w );
}

void draw_compass( const catacurses::window &w )
{
    // compass panel
    const std::string title = _( "Compass" );
    decorate_panel( title, w );
    // const std::string compass = "";
    mvwprintz( w, 1, 5, c_light_gray, "                      " );
    mvwprintz( w, 2, 5, c_light_gray, "     --._ | _,--      " );
    mvwprintz( w, 3, 5, c_light_gray, "       /  N  \\       " );
    mvwprintz( w, 4, 5, c_light_gray, "    --< W + E >--     " );
    mvwprintz( w, 5, 5, c_light_gray, "       \\_ S _/       " );
    mvwprintz( w, 6, 5, c_light_gray, "     --'  |  `--      " );
    mvwprintz( w, 7, 5, c_light_gray, "                      " );

    //    mvwprintz( w, 1, 5, c_light_gray, "         oZz         " );
    //    mvwprintz( w, 2, 5, c_light_gray, "  Zdd __|   |__ kzz  " );
    //    mvwprintz( w, 3, 5, c_light_gray, "          N          " );
    //    mvwprintz( w, 4, 5, c_light_gray, "  Zzk   W + E   dZzz " );
    //    mvwprintz( w, 5, 5, c_light_gray, "      __  S  __      " );
    //    mvwprintz( w, 6, 5, c_light_gray, "  ooZ   |   |   zzk  " );
    //    mvwprintz( w, 7, 5, c_light_gray, "         Zzz         " );

    wrefresh( w );
}


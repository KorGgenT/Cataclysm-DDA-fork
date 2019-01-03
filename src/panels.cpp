#include "panels.h"

#include "cata_utility.h"
#include "color.h"
#include "cursesdef.h"
#include "game.h"
#include "gun_mode.h"
#include "item.h"
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

static const trait_id trait_SELFAWARE( "SELFAWARE" );

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

void draw_character( player &u, const catacurses::window &w )
{
    // character panel
    static const std::string title = _( "Character" );
    decorate_panel( title, w );

    std::pair<nc_color, int> morale_pair = morale_stat( u );
    mvwprintz( w,  1, 1,  c_light_gray, "%s", _( "Head    :" ) );
    mvwprintz( w,  1, 11,  stat_color2( int( u.hp_cur[hp_head] ) ),
               std::to_string( u.hp_cur[hp_head] ) );

    mvwprintz( w,  2, 1,  c_light_gray, "%s %d", _( "L_arm   :" ), u.hp_cur[hp_arm_l] );
    mvwprintz( w,  3, 1,  c_light_gray, "%s %d", _( "L_leg   :" ), u.hp_cur[hp_leg_l] );
    mvwprintz( w,  5, 1,  c_light_gray, "%s %d", _( "Sound   :" ), u.volume );
    mvwprintz( w,  6, 1,  c_light_gray, "%s %d", _( "Stamina :" ), int( u.stamina / 10 ) );
    mvwprintz( w,  7, 1,  c_light_gray, "%s %d", _( "Focus   :" ), u.focus_pool );
    mvwprintz( w,  9, 1,  c_light_gray, "%s %d", _( "Strength:" ), u.str_cur );
    mvwprintz( w, 10, 1,  c_light_gray, "%s %d", _( "Intel   :" ), u.int_cur );

    mvwprintz( w,  1, 15, c_light_gray, "%s %d", _( "|  Torso   :" ), u.hp_cur[hp_torso] );
    mvwprintz( w,  2, 15, c_light_gray, "%s %d", _( "|  R_arm   :" ), u.hp_cur[hp_arm_r] );
    mvwprintz( w,  3, 15, c_light_gray, "%s %d", _( "|  R_leg   :" ), u.hp_cur[hp_leg_r] );
    mvwprintz( w,  5, 15, c_light_gray, "%s", _( "|  Morale  :" ), u.get_morale_level() );
    mvwprintz( w,  5, 27, morale_pair.first, std::to_string( morale_pair.second ) );

    mvwprintz( w,  6, 15, c_light_gray, "%s %d", _( "|  Speed   :" ), u.get_speed() );
    mvwprintz( w,  7, 15, c_light_gray, "%s %d", _( "|  move    :" ), u.movecounter );
    mvwprintz( w,  9, 15, c_light_gray, "%s %d", _( "|  Dexter  :" ), u.dex_cur );
    mvwprintz( w, 10, 15, c_light_gray, "%s %d", _( "|  Percep  :" ), u.per_cur );

    // stat_color( get_speed_bonus() ),

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
    mvwprintz( w, 6, 1, c_light_gray, _( "Moon    : ???" ) );
    mvwprintz( w, 8, 1, c_light_gray, _( "Temp    : ???" ) );
    mvwprintz( w, 9, 1, c_light_gray, _( "Rad     : ???" ) );
    wrefresh( w );
}

// static std::string print_gun_mode( const player &u );
void draw_modifiers( const player &u, const catacurses::window &w )
{
    // modifiers panel
    const std::string title = _( "Modifiers" );
    decorate_panel( title, w );

    std::pair<nc_color, std::string> hunger_pair = hunger_stat( u );
    std::pair<nc_color, std::string> thirst_pair = thirst_stat( u );
    std::pair<nc_color, std::string> rest_pair = rest_stat( u );
    std::pair<nc_color, std::string> temp_pair = temp_stat( u );
    std::pair<nc_color, std::string> pain_pair = pain_stat( u );
    mvwprintz( w, 1,  1,  c_light_gray, _( "Weapon  :" ) );
    mvwprintz( w, 2,  1,  c_light_gray, _( "Food    :" ) );
    mvwprintz( w, 3,  1,  c_light_gray, _( "Drink   :" ) );
    mvwprintz( w, 4,  1,  c_light_gray, _( "Rest    :" ) );
    mvwprintz( w, 5,  1,  c_light_gray, _( "Pain    :" ) );
    mvwprintz( w, 6,  1,  c_light_gray, _( "Heat    :" ) );
    mvwprintz( w, 7,  1,  c_light_gray, _( "Drug    :" ) );
    mvwprintz( w, 8,  1,  c_light_gray, _( "Head    :" ) );
    mvwprintz( w, 9,  1,  c_light_gray, _( "Torso   :" ) );
    mvwprintz( w, 10, 1,  c_light_gray, _( "Legs    :" ) );
    mvwprintz( w, 11, 1,  c_light_gray, _( "Feet    :" ) );

    mvwprintz( w, 1,  11, c_light_gray, u.weapname().c_str() );
    mvwprintz( w, 2,  11, hunger_pair.first, hunger_pair.second );
    mvwprintz( w, 3,  11, thirst_pair.first, thirst_pair.second );
    mvwprintz( w, 4,  11, rest_pair.first, rest_pair.second );
    mvwprintz( w, 5,  11, pain_pair.first, pain_pair.second );
    mvwprintz( w, 6,  11, temp_pair.first, temp_pair.second );

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
    mvwprintz( w, 1,   1, c_light_gray, "Detected : No  |  Total : 0" );
    mvwprintz( w, 2,  11, c_light_gray, " " );
    mvwprintz( w, 4,  11, c_light_gray, " _   |   _ " );
    mvwprintz( w, 5,  11, c_light_gray, "  \\_ N _/ " );
    mvwprintz( w, 6,  11, c_light_gray, "  /  |  \\ " );
    mvwprintz( w, 7,  11, c_light_gray, " W --+-- E " );
    mvwprintz( w, 8,  11, c_light_gray, "  \\_ | _/ " );
    mvwprintz( w, 9,  11, c_light_gray, " _/  S  \\_" );
    mvwprintz( w, 10, 11, c_light_gray, "     |     " );

    wrefresh( w );
}

//std::string print_gun_mode( const player &u )
//{
//    auto m = u.weapon.gun_current_mode();
//    if( m ) {
//        if( m.melee() || !m->is_gunmod() ) {
//            if( u.ammo_location && u.weapon.can_reload_with( u.ammo_location->typeId() ) ) {
//                return string_format( "%s (%d)", u.weapname().c_str(),
//                                      u.ammo_location->charges );
//            }
//            return string_format( m.name().empty() ? "%s" : "%s (%s)",
//                                  u.weapname().c_str(), m.name() );
//        } else {
//            return string_format( "%s (%i/%i)", m->tname().c_str(),
//                                  m->ammo_remaining(), m->ammo_capacity() );
//        }
//    } else {
//        return u.weapname();
//    }
//}


nc_color stat_color2( int stat )
{
    nc_color statcolor = c_white;
    if( stat >= 70 ) {
        statcolor = c_green;
    } else if( stat >= 50 ) {
        statcolor = c_brown_red;
    } else if( stat >= 30 ) {
        statcolor = c_red;
    }

    return statcolor;
}


std::pair<nc_color, int> morale_stat( const player &u )
{
    const int morale_int = u.get_morale_level();
    nc_color morale_color = c_white;
    if( morale_int >= 10 ) {
        morale_color = c_green;
    } else if( morale_int <= -10 ) {
        morale_color = c_red;
    }
    return std::make_pair( morale_color, morale_int );
}

std::pair<nc_color, std::string> hunger_stat( const player &u )
{
    std::string hunger_string = "";
    nc_color hunger_color = c_yellow;
    if( u.get_hunger() >= 300 && u.get_starvation() > 2500 ) {
        hunger_color = c_red;
        hunger_string = _( "Starving!" );
    } else if( u.get_hunger() >= 300 && u.get_starvation() > 1100 ) {
        hunger_color = c_light_red;
        hunger_string = _( "Near starving" );
    } else if( u.get_hunger() > 250 ) {
        hunger_color = c_light_red;
        hunger_string = _( "Famished" );
    } else if( u.get_hunger() > 100 ) {
        hunger_color = c_yellow;
        hunger_string = _( "Very hungry" );
    } else if( u.get_hunger() > 40 ) {
        hunger_color = c_yellow;
        hunger_string = _( "Hungry" );
    } else if( u.get_hunger() < -60 ) {
        hunger_color = c_green;
        hunger_string = _( "Engorged" );
    } else if( u.get_hunger() < -20 ) {
        hunger_color = c_green;
        hunger_string = _( "Sated" );
    } else if( u.get_hunger() < 0 ) {
        hunger_color = c_green;
        hunger_string = _( "Full" );
    }
    return std::make_pair( hunger_color, hunger_string );
}

std::pair<nc_color, std::string> thirst_stat( const player &u )
{
    std::string hydration_string = "";
    nc_color hydration_color = c_yellow;
    if( u.get_thirst() > 520 ) {
        hydration_color = c_light_red;
        hydration_string = _( "Parched" );
    } else if( u.get_thirst() > 240 ) {
        hydration_color = c_light_red;
        hydration_string = _( "Dehydrated" );
    } else if( u.get_thirst() > 80 ) {
        hydration_color = c_yellow;
        hydration_string = _( "Very Thirsty" );
    } else if( u.get_thirst() > 40 ) {
        hydration_color = c_yellow;
        hydration_string = _( "Thirsty" );
    } else if( u.get_thirst() < -60 ) {
        hydration_color = c_green;
        hydration_string = _( "Turgid" );
    } else if( u.get_thirst() < -20 ) {
        hydration_color = c_green;
        hydration_string = _( "Hydrated" );
    } else if( u.get_thirst() < 0 ) {
        hydration_color = c_green;
        hydration_string = _( "Slaked" );
    }
    return std::make_pair( hydration_color, hydration_string );
}

std::pair<nc_color, std::string> rest_stat( const player &u )
{
    std::string rest_string = "";
    nc_color rest_color = c_yellow;
    if( u.get_fatigue() > EXHAUSTED ) {
        rest_color = c_red;
        rest_string = _( "Exhausted" );
    } else if( u.get_fatigue() > DEAD_TIRED ) {
        rest_color = c_light_red;
        rest_string = _( "Dead tired" );
    } else if( u.get_fatigue() > TIRED ) {
        rest_color = c_yellow;
        rest_string = _( "Tired" );
    }
    return std::make_pair( rest_color, rest_string );
}

std::pair<nc_color, std::string> temp_stat( const player &u )
{
    /// Find hottest/coldest bodypart
    // Calculate the most extreme body temperatures
    int current_bp_extreme = 0;
    int conv_bp_extreme = 0;
    for( int i = 0; i < num_bp ; i++ ) {
        if( abs( u.temp_cur[i] - BODYTEMP_NORM ) > abs( u.temp_cur[current_bp_extreme] - BODYTEMP_NORM ) ) {
            current_bp_extreme = i;
        }
        if( abs( u.temp_conv[i] - BODYTEMP_NORM ) > abs( u.temp_conv[conv_bp_extreme] - BODYTEMP_NORM ) ) {
            conv_bp_extreme = i;
        }
    }

    // printCur the hottest/coldest bodypart, and if it is rising or falling in temperature
    std::string temp_string = "";
    nc_color temp_color = c_yellow;
    if( u.temp_cur[current_bp_extreme] >         BODYTEMP_SCORCHING ) {
        temp_color  = c_red;
        temp_string = _( "Scorching!" );
    } else if( u.temp_cur[current_bp_extreme] >  BODYTEMP_VERY_HOT ) {
        temp_color  = c_light_red;
        temp_string = _( "Very hot!" );
    } else if( u.temp_cur[current_bp_extreme] >  BODYTEMP_HOT ) {
        temp_color  = c_yellow;
        temp_string =  _( "Warm" );
    } else if( u.temp_cur[current_bp_extreme] >  BODYTEMP_COLD ) {
        temp_color  = c_green;
        temp_string = _( "Comfortable" );
    } else if( u.temp_cur[current_bp_extreme] >  BODYTEMP_VERY_COLD ) {
        temp_color  = c_light_blue;
        temp_string = _( "Chilly" );
    } else if( u.temp_cur[current_bp_extreme] >  BODYTEMP_FREEZING ) {
        temp_color  = c_cyan;
        temp_string = _( "Very cold!" );
    } else if( u.temp_cur[current_bp_extreme] <= BODYTEMP_FREEZING ) {
        temp_color  = c_blue;
        temp_string = _( "Freezing!" );
    }
    return std::make_pair( temp_color, temp_string );
}

std::pair<nc_color, std::string> pain_stat( const player &u )
{
    nc_color pain_color = c_yellow;
    std::string pain_string = "";
    // get pain color
    if( u.get_perceived_pain() >= 60 ) {
        pain_color = c_red;
    } else if( u.get_perceived_pain() >= 40 ) {
        pain_color = c_light_red;
    }
    // get pain string
    if( u.has_trait( trait_SELFAWARE ) && u.get_perceived_pain() > 0 ) {
        pain_string = u.get_perceived_pain();
    } else if( u.get_perceived_pain() > 0 ) {
        pain_string = u.get_pain_description();
    }
    return std::make_pair( pain_color, pain_string );
}

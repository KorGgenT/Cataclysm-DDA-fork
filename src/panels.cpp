#include "panels.h"

#include "cata_utility.h"
#include "color.h"
#include "cursesdef.h"
#include "game.h"
#include "gun_mode.h"
#include "item.h"
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
#include <typeinfo>

#if (defined TILES || defined _WIN32 || defined WINDOWS)
#include "cursesport.h"
#endif

static const trait_id trait_SELFAWARE( "SELFAWARE" );

// ===============================
// panels code
// ===============================
void draw_panel_adm( const catacurses::window &w )
{

    input_context ctxt( "PANEL_MGMT" );
    ctxt.register_action( "HELP_KEYBINDINGS" );
    ctxt.register_action( "QUIT" );
    ctxt.register_action( "UP" );
    ctxt.register_action( "DOWN" );
    ctxt.register_action( "LEFT" );
    ctxt.register_action( "RIGHT" );
    ctxt.register_action( "MOVE_PANEL" );

    int index = 1;
    int counter = 0;
    bool selected = false;
    struct w_map w_arr[10];
    int savedindex = 0;
    int targetindex = 0;
    bool redraw = true;
    bool exit = false;

    while( !exit ) {
        if( redraw ) {
            redraw = false;
            werase( w );
            static const std::string title = _( "panel admin" );
            decorate_panel( title, w );
            for( int i = 0; i < ( int )g->win_map.size(); i++ ) {
                mvwprintz( w, i + 1, 4,
                           g->win_map[ i ].toggle  ?
                           savedindex == i && selected ? c_yellow : c_white : c_dark_gray,
                           selected && index - 1 == i ? " %s" : "%s",
                           g->win_map[ i ].name );
            }
            mvwprintz( w, index, 1, c_yellow, ">>" );
            mvwvline( w, 1, 10, 0, 13 );
            mvwprintz( w, 1, 13, c_white, "right: toggle panels on/off" );
            mvwprintz( w, 2, 13, c_white, "enter: change display order" );
        }
        wrefresh( w );

        const std::string action = ctxt.handle_input();
        if( action == "UP" ) {
            if( !( index <= 1 ) ) {
                index -= 1;
                redraw = true;
            }
        } else if( action == "DOWN" ) {
            if( !( index >= ( int )g->win_map.size() ) ) {
                index += 1;
                redraw = true;
            }
        } else if( action == "MOVE_PANEL" ) {
            counter += 1;
            // source window from the swap
            if( counter == 1 ) {
                // saving win1
                selected = true;
                w_arr[0] = g->win_map.at( index - 1 );
                savedindex = index - 1;
            }
            // dest window for the swap
            if( counter == 2 ) {
                // saving win2
                targetindex = index - 1;
                w_arr[1] = g->win_map.at( index - 1 );

                int distance = targetindex - savedindex;
                int step_dir = distance > 0 ? 1 : -1;
                for( int i = savedindex; i != targetindex; i += step_dir ) {
                    std::swap( g->win_map[i], g->win_map[i + step_dir] );
                }

                //for( int i = 0; i < ( int )g->win_map.size(); i++ ) {
                //    // ex: msg
                //    if( g->win_map[ i ].win == w_arr[0].win ) {
                //        g->win_map[ i ] = w_arr[1];
                //    }
                //    // ex: stat
                //    if( g->win_map[ i ].win == w_arr[1].win && !( i == savedindex ) ) {
                //        g->win_map[ i ] = w_arr[0];
                //    }
                //}

                int y_top = 0;
                for( int i = 0; i < ( int )g->win_map.size(); i++ ) {
                    // stack the panels
                    if( g->win_map[ i ].toggle ) {
                        g->win_map[ i ].win.get<cata_cursesport::WINDOW>()->y = y_top;
                        y_top += g->win_map[ i ].win.get<cata_cursesport::WINDOW>()->height;
                    }
                    // reinit map
                    if( g->win_map[ i ].name == "map" ) {
                        g->w_pixel_minimap.get<cata_cursesport::WINDOW>()->y =
                            g->win_map[ i ].win.get<cata_cursesport::WINDOW>()->y;
                        g->reinitmap = true;
                    }
                }
                counter = 0;
                selected = false;
            }
            redraw = true;
        } else if( action == "RIGHT" ) {
            // toggling panels
            g->win_map[ index - 1 ].toggle = !g->win_map[ index - 1 ].toggle;

            int y_top = 0;
            for( int i = 0; i < ( int )g->win_map.size(); i++ ) {
                if( g->win_map[ i ].toggle ) {
                    g->win_map[ i ].win.get<cata_cursesport::WINDOW>()->y = y_top;
                    y_top += g->win_map[ i ].win.get<cata_cursesport::WINDOW>()->height;
                    // reinit map
                    if( g->win_map[ i ].name == "map" ) {
                        g->w_pixel_minimap.get<cata_cursesport::WINDOW>()->y =
                            g->win_map[ i ].win.get<cata_cursesport::WINDOW>()->y;
                        g->reinitmap = true;
                    }
                }
            }
            redraw = true;
        } else if( action == "QUIT" ) {
            exit = true;
            g->show_panel_adm = false;
        }
    }
}

void draw_limb( player &u, const catacurses::window &w )
{
    werase( w );
    // limb panel

    mvwprintz( w,  1, 1,  c_light_gray, _( "Head  :" ) );
    mvwprintz( w,  1, 18, c_light_gray, _( "Torso :" ) );
    mvwprintz( w,  2, 1,  c_light_gray, _( "L_Arm :" ) );
    mvwprintz( w,  2, 18, c_light_gray, _( "R_Arm :" ) );
    mvwprintz( w,  3, 1,  c_light_gray, _( "L_Leg :" ) );
    mvwprintz( w,  3, 18, c_light_gray, _( "R_Leg :" ) );

    const auto &head =  get_hp_bar( u.hp_cur[hp_head], u.hp_max[hp_head] );
    const auto &torso = get_hp_bar( u.hp_cur[hp_torso], u.hp_max[hp_torso] );
    const auto &arml =  get_hp_bar( u.hp_cur[hp_arm_l], u.hp_max[hp_arm_l] );
    const auto &armr =  get_hp_bar( u.hp_cur[hp_arm_r], u.hp_max[hp_arm_r] );
    const auto &legl =  get_hp_bar( u.hp_cur[hp_leg_l], u.hp_max[hp_leg_l] );
    const auto &legr =  get_hp_bar( u.hp_cur[hp_leg_r], u.hp_max[hp_leg_r] );

    mvwprintz( w,  1, 9,  stat_color( u.hp_cur[hp_head] ),  "%s", head.first );
    mvwprintz( w,  1, 26, stat_color( u.hp_cur[hp_torso] ), "%s", torso.first );
    mvwprintz( w,  2, 9,  stat_color( u.hp_cur[hp_arm_l] ), "%s", arml.first );
    mvwprintz( w,  2, 26, stat_color( u.hp_cur[hp_arm_r] ), "%s", armr.first );
    mvwprintz( w,  3, 9,  stat_color( u.hp_cur[hp_leg_l] ), "%s", legl.first );
    mvwprintz( w,  3, 26, stat_color( u.hp_cur[hp_leg_r] ), "%s", legr.first );
    wrefresh( w );
}

void draw_char( player &u, const catacurses::window &w )
{
    werase( w );
    std::pair<nc_color, int> morale_pair = morale_stat( u );
    mvwprintz( w,  1, 1,  c_light_gray, _( "Sound :" ) );
    mvwprintz( w,  2, 1,  c_light_gray, _( "Stam  :" ) );
    mvwprintz( w,  3, 1,  c_light_gray, _( "Focus :" ) );
    mvwprintz( w,  1, 18, c_light_gray, _( "Mood  :" ) );
    mvwprintz( w,  2, 18, c_light_gray, _( "Speed :" ) );
    mvwprintz( w,  3, 18, c_light_gray, _( "move  :" ) );

    mvwprintz( w,  1, 9, c_light_gray, "%s", u.volume );
    mvwprintz( w,  2, 9, stat_color( u.stamina / 10 ), "%s", u.stamina / 10 );
    mvwprintz( w,  3, 9, stat_color( u.focus_pool ), "%s", u.focus_pool );
    mvwprintz( w,  1, 26, morale_pair.first, "%s", morale_pair.second );
    mvwprintz( w,  2, 26, stat_color( u.get_speed() ), "%s", u.get_speed() );
    mvwprintz( w,  3, 26, c_light_gray, "%s", u.movecounter );
    wrefresh( w );
}

void draw_stat( player &u, const catacurses::window &w )
{
    werase( w );
    mvwprintz( w, 1, 1,  c_light_gray, _( "Str:" ) );
    mvwprintz( w, 1, 9, c_light_gray,  _( "Int:" ) );
    mvwprintz( w, 1, 18, c_light_gray, _( "Dex:" ) );
    mvwprintz( w, 1, 26, c_light_gray, _( "Per:" ) );

    mvwprintz( w, 1, 5,  stat_color( u.str_cur ), "%s", u.str_cur );
    mvwprintz( w, 1, 13, stat_color( u.int_cur ), "%s", u.int_cur );
    mvwprintz( w, 1, 22, stat_color( u.dex_cur ), "%s", u.dex_cur );
    mvwprintz( w, 1, 30, stat_color( u.per_cur ), "%s", u.per_cur );
    wrefresh( w );
}

void draw_env1( const player &u, const catacurses::window &w )
{
    werase( w );
    // display location
    const oter_id &cur_ter = overmap_buffer.ter( u.global_omt_location() );
    mvwprintz( w, 0, 1, c_light_gray, "Place: " );
    wprintz( w, c_white, utf8_truncate( cur_ter->get_name(), getmaxx( w ) - 13 ) );
    // display weather
    if( g->get_levz() < 0 ) {
        mvwprintz( w, 1, 1, c_light_gray, _( "Sky  : Underground" ) );
    } else {
        mvwprintz( w, 1, 1, c_light_gray, _( "Sky  :" ) );
        wprintz( w, weather_data( g->weather ).color, " %s",
                 weather_data( g->weather ).name.c_str() );
    }
    // display lighting
    const auto ll = get_light_level( g->u.fine_detail_vision_mod() );
    mvwprintz( w, 2, 1, c_light_gray, "%s ", _( "Light:" ) );
    wprintz( w, ll.second, ll.first.c_str() );

    // display date
    mvwprintz( w, 3, 1, c_light_gray, _( "Date : %s, day %d" ),
               calendar::name_season( season_of_year( calendar::turn ) ),
               day_of_season<int>( calendar::turn ) + 1 );

    // display time
    if( u.has_watch() ) {
        mvwprintz( w, 4, 1, c_light_gray, _( "Time : %s" ),
                   to_string_time_of_day( calendar::turn ) );
    } else if( g->get_levz() >= 0 ) {
        mvwprintz( w, 4, 1, c_light_gray, _( "Time : %s" ), time_approx() );
    } else {
        mvwprintz( w, 4, 1, c_light_gray, _( "Time : ???" ) );
    }
    wrefresh( w );
}
void draw_env2( const player &u, const catacurses::window &w )
{
    werase( w );
    mvwprintz( w, 1, 1, c_light_gray, _( "Moon : %s" ), get_moon( ) );
    mvwprintz( w, 2, 1, c_light_gray, _( "Temp : %s" ), get_temp( u ) );
    mvwprintz( w, 3, 1, c_light_gray, _( "wind : -" ) );
    mvwprintz( w, 4, 1, c_light_gray, _( "Rad  : -" ) );
    wrefresh( w );
}

void draw_mod1( const player &u, const catacurses::window &w )
{
    werase( w );
    std::pair<nc_color, std::string> hunger_pair = hunger_stat( u );
    std::pair<nc_color, std::string> thirst_pair = thirst_stat( u );
    std::pair<nc_color, std::string> rest_pair = rest_stat( u );
    std::pair<nc_color, std::string> temp_pair = temp_stat( u );
    std::pair<nc_color, std::string> pain_pair = pain_stat( u );
    mvwprintz( w, 0,  1,  c_light_gray, _( "Arm  :" ) );
    mvwprintz( w, 1,  1,  c_light_gray, _( "Food :" ) );
    mvwprintz( w, 2,  1,  c_light_gray, _( "Drink:" ) );
    mvwprintz( w, 3,  1,  c_light_gray, _( "Rest :" ) );
    mvwprintz( w, 4,  1,  c_light_gray, _( "Pain :" ) );
    mvwprintz( w, 5,  1,  c_light_gray, _( "Heat :" ) );
    mvwprintz( w, 0,  8, c_light_gray, u.weapname().c_str() );
    mvwprintz( w, 1,  8, hunger_pair.first, hunger_pair.second );
    mvwprintz( w, 2,  8, thirst_pair.first, thirst_pair.second );
    mvwprintz( w, 3,  8, rest_pair.first, rest_pair.second );
    mvwprintz( w, 4,  8, pain_pair.first, pain_pair.second );
    mvwprintz( w, 5,  8, temp_pair.first, temp_pair.second );
    wrefresh( w );
}

void draw_mod2( const player &u, const catacurses::window &w )
{
    werase( w );
    mvwprintz( w, 1, 1, c_light_gray, _( "Temp : %s" ), get_temp( u ) );
    mvwprintz( w, 2, 1, c_light_gray, _( "Drug : too much cafeine O_O" ) );
    mvwprintz( w, 3, 1, c_light_gray, _( "Head :" ) );
    mvwprintz( w, 4, 1, c_light_gray, _( "Torso:" ) );
    mvwprintz( w, 5, 1, c_light_gray, _( "Arms :" ) );
    mvwprintz( w, 6, 1, c_light_gray, _( "Legs :" ) );
    mvwprintz( w, 7, 1, c_light_gray, _( "Feet :" ) );

    mvwprintz( w, 3, 8, c_light_gray, get_armor( u, bp_head, w ) );
    mvwprintz( w, 4, 8, c_light_gray, get_armor( u, bp_torso, w ) );
    mvwprintz( w, 5, 8, c_light_gray, get_armor( u, bp_arm_r, w ) );
    mvwprintz( w, 6, 8, c_light_gray, get_armor( u, bp_leg_r, w ) );
    mvwprintz( w, 7, 8, c_light_gray, get_armor( u, bp_foot_r, w ) );
    wrefresh( w );
}

void draw_messages( const catacurses::window &w )
{
    werase( w );
    int line = getmaxy( w ) - 2;
    int maxlength = getmaxx( w );
    Messages::display_messages( w, 0, 1 /*topline*/, maxlength - 1, line );
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
    //    for( int i = 0; i < ( int )g->win_map.size(); i++ ) {
    //        if( g->win_map[ i ].name == "map" && g->win_map[ i ].toggle ) {
    //            wrefresh( w );
    //            g->draw_pixel_minimap();
    //        }
    //    }
    wrefresh( w );
    g->draw_pixel_minimap();
}

void draw_compass( const catacurses::window &w )
{
    werase( w );
    g->mon_info( w );

    // const std::string compass = "";
    //    mvwprintz( w, 1,   1, c_light_gray, "Detected : No  |  Total : 0" );
    //    mvwprintz( w, 2,  11, c_light_gray, " " );
    //    mvwprintz( w, 4,  11, c_light_gray, " _   |   _ " );
    //    mvwprintz( w, 5,  11, c_light_gray, "  \\_ N _/ " );
    //    mvwprintz( w, 6,  11, c_light_gray, "  /  |  \\ " );
    //    mvwprintz( w, 7,  11, c_light_gray, " W --+-- E " );
    //    mvwprintz( w, 8,  11, c_light_gray, "  \\_ | _/ " );
    //    mvwprintz( w, 9,  11, c_light_gray, " _/  S  \\_" );
    //    mvwprintz( w, 10, 11, c_light_gray, "     |     " );

    wrefresh( w );
}


// ====================================
// panels prettify and helper functions
// ====================================

void decorate_panel( const std::string name, const catacurses::window &w )
{
    werase( w );
    draw_border( w );

    static const char *title_prefix = "< ";
    const std::string title = name;
    static const char *title_suffix = " >";
    static const std::string full_title = string_format( "%s%s%s",
                                          title_prefix, title, title_suffix );
    const int start_pos = center_text_pos( full_title.c_str(), 0, getmaxx( w ) - 1 );
    mvwprintz( w, 0, start_pos, c_white, title_prefix );
    wprintz( w, c_blue, title );
    wprintz( w, c_white, title_suffix );
}

std::string get_temp( const player &u )
{
    std::string temp = "";
    if( u.has_item_with_flag( "THERMOMETER" ) ||
        u.has_bionic( bionic_id( "bio_meteorologist" ) ) ) {
        temp = print_temperature( g->get_temperature( u.pos() ) ).c_str();
    }
    return temp;
}

std::string get_moon()
{
    std::string moon = "";
    const int iPhase = static_cast<int>( get_moon_phase( calendar::turn ) );
    if( iPhase == 0 ) {
        moon = "New moon";
    }
    if( iPhase == 1 ) {
        moon  = "Waxing crescent";
    }
    if( iPhase == 2 ) {
        moon  = "Half moon";
    }
    if( iPhase == 3 ) {
        moon  = "Waxing gibbous";
    }
    if( iPhase == 4 ) {
        moon  = "Full moon";
    }
    if( iPhase == 5 ) {
        moon  = "Waning gibbous";
    }
    if( iPhase == 6 ) {
        moon  = "Half moon";
    }
    if( iPhase == 7 ) {
        moon  = "Waning crescent";
    }
    if( iPhase == 8 ) {
        moon  = "Dark moon";
    }
    return moon;
}

std::string time_approx()
{
    const int iHour = hour_of_day<int>( calendar::turn );
    std::string time_approx = "";
    if( iHour >= 22 ) {
        time_approx = "Around midnight";
    } else if( iHour >= 20 ) {
        time_approx = "It's getting darker";
    } else if( iHour >= 16 ) {
        time_approx = "This is the Evening";
    } else if( iHour >= 13 ) {
        time_approx = "In the afternoon";
    } else if( iHour >= 11 ) {
        time_approx = "Around noon";
    } else if( iHour >= 8 ) {
        time_approx = "In the Morning";
    } else if( iHour >= 5 ) {
        time_approx = "Around Dawn";
    } else if( iHour >= 0 ) {
        time_approx = "Dead of Night";
    }
    return time_approx;
}

nc_color stat_color( int stat )
{
    nc_color statcolor = c_light_gray;
    if( stat >= 75 ) {
        statcolor = c_green;
    } else if( stat >= 50 ) {
        statcolor = c_yellow;
    } else if( stat >= 25 ) {
        statcolor = c_red;
    } else if( stat >= 0 ) {
        statcolor = c_magenta;
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

std::string get_armor( const player &u, body_part bp, const catacurses::window &w )
{
    for( std::list<item>::const_iterator it = u.worn.end(); it != u.worn.begin(); ) {
        --it;
        if( it->covers( bp ) ) {
            std::string temp = it->tname( 1, false );
            if( ( int )temp.length() > getmaxx( w ) - 9 ) {
                // string minus window size.x and label
                temp = temp.substr( 0, getmaxx( w ) - 11 );
                temp += "..";
                return temp;
            }
            return temp;
        }
    }
    return "-";
}

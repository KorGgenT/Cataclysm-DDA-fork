#include "panels.h"

#include "cata_utility.h"
#include "color.h"
#include "cursesdef.h"
#include "game.h"
#include "output.h"
#include "player.h"


void draw_character( const player &u, const catacurses::window &w_panel1 )
{

    werase( w_panel1 );
    draw_border( w_panel1 );

    static const char *title_prefix = "< ";
    static const char *title = _( "Character" );
    static const char *title_suffix = " >";
    static const std::string full_title = string_format( "%s%s%s", title_prefix, title, title_suffix );
    const int start_pos = center_text_pos( full_title.c_str(), 0, getmaxx( w_panel1 ) - 1 );
    mvwprintz( w_panel1, 0, start_pos, c_white, title_prefix );
    wprintz( w_panel1, c_green, title );
    wprintz( w_panel1, c_white, title_suffix );

    const int dy  = 1;
    static std::array<body_part, 7> part = {{
            bp_head, bp_torso, bp_arm_l, bp_arm_r, bp_leg_l, bp_leg_r, num_bp
        }
    };
    for( size_t i = 0; i < part.size(); i++ ) {
        const std::string str = ( i == part.size() - 1 ) ?
                                _( "POWER" ) : body_part_hp_bar_ui_text( part[i] );
        wmove( w_panel1, ( i + 1 ) * dy, 1 );
        wprintz( w_panel1, u.limb_color( part[i], true, true, true ), " " );
        wprintz( w_panel1, u.limb_color( part[i], true, true, true ),
                 str + " : " + std::to_string( u.hp_cur[i] ) );

    }

    wrefresh( w_panel1 );
}

void draw_zmypanel()
{
    //    werase( w_panel1l1 );
    //    draw_border( w_panel1l1 );

    //    static const char *title_prefix = "< ";
    //    static const char *title = _( "Look Around" );
    //    static const char *title_suffix = " >";
    //    static const std::string full_title = string_format( "%s%s%s", title_prefix, title, title_suffix );
    //    const int start_pos = center_text_pos( full_title.c_str(), 0, getmaxx( w_panel1l1 ) - 1 );
    //    mvwprintz( w_panel1l1, 0, start_pos, c_white, title_prefix );
    //    wprintz( w_panel1l1, c_green, title );
    //    wprintz( w_panel1l1, c_white, title_suffix );

    //    // const bool wide = ( getmaxy( w_HP ) == 7 );
    //    // const bool is_self_aware = u.has_trait( trait_SELFAWARE );
    //    const int hpx = 0;
    //    const int hpy = 1;
    //    const int dy  = 1;
    //    for( int i = 0; i < num_hp_parts; i++ ) {
    //        wmove( w_panel1l1, i * dy + hpy, hpx );

    //        static auto print_symbol_num = []( const catacurses::window & w,
    //        int num, const std::string & sym, const nc_color color ) {
    //            while( num-- > 0 ) {
    //                wprintz( w, color, sym.c_str() );
    //            }
    //        };

    //        if( u.hp_cur[i] == 0 && ( i >= hp_arm_l && i <= hp_leg_r ) ) {
    //            //Limb is broken
    //            std::string limb = "~~%~~";
    //            nc_color color = c_light_red;

    //            const auto bp = u.hp_to_bp( static_cast<hp_part>( i ) );
    //            if( u.worn_with_flag( "SPLINT", bp ) ) {
    //                static const efftype_id effect_mending( "mending" );
    //                const auto &eff = u.get_effect( effect_mending, bp );
    //                const int mend_perc = eff.is_null() ? 0.0 : 100 * eff.get_duration() / eff.get_max_duration();

    //                //                if( u.is_self_aware ) {
    //                //                    limb = string_format( "=%2d%%=", mend_perc );
    //                //                    color = c_blue;
    //                //                } else {
    //                const int num = mend_perc / 20;
    //                print_symbol_num( w_panel1l1, num, "#", c_blue );
    //                print_symbol_num( w_panel1l1, 5 - num, "=", c_blue );
    //                continue;
    //                // }
    //            }

    //            wprintz( w_panel1l1, color, limb );
    //            continue;
    //        }

    //        const auto &hp = get_hp_bar( u.hp_cur[i], u.hp_max[i] );

    //        //        if( is_self_aware ) {
    //        //            wprintz( w_panel1l1, hp.second, "%3d  ", u.hp_cur[i] );
    //        //        } else {
    //        wprintz( w_panel1l1, hp.second, hp.first );

    //        //Add the trailing symbols for a not-quite-full health bar
    //        print_symbol_num( w_panel1l1, 5 - static_cast<int>( hp.first.size() ), ".", c_white );
    //        //        }
    //    }

    //    static std::array<body_part, 7> part = {{
    //            bp_head, bp_torso, bp_arm_l, bp_arm_r, bp_leg_l, bp_leg_r, num_bp
    //        }
    //    };
    //    for( size_t i = 0; i < part.size(); i++ ) {
    //        const std::string str = ( i == part.size() - 1 ) ?
    //                                _( "POWER" ) : body_part_hp_bar_ui_text( part[i] );
    //        wmove( w_panel1l1, ( i + 1 ) * dy, 1 );
    //        wprintz( w_panel1l1, u.limb_color( part[i], true, true, true ), " " );
    //        wprintz( w_panel1l1, u.limb_color( part[i], true, true, true ), str );

    //    }
    //    wrefresh( w_panel1l1 );
}

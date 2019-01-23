#include "catch/catch.hpp"
#include "game.h"
#include "player.h"
#include "activity_handlers.h"
#include "itype.h"
#include "map.h"
#include "map_helpers.h"
#include "overmapbuffer.h"
#include "iexamine.h"
#include "map_iterator.h"
#include "test_statistics.h"

std::pair<std::string, int> global_winner = std::make_pair( "", 0 );
std::pair<std::string, int> global_loser = std::make_pair( "", 2147483647 );
std::map<std::string, bool> global_no_kcal;
std::string global_winner_contents = "";
std::map<itype_id, int> global_contents;

struct forage_test_data {
    statistics<int> calories;
    statistics<bool> forests;
};

void i_clear_adjacent( const tripoint &p )
{
    auto close_trip = g->m.points_in_radius( p, 1 );
    for( const auto &trip : close_trip ) {
        g->m.i_clear( trip );
    }
}

// gives the calories and items a player would harvest from an underbrush, given the items_location
std::pair<int, int> forage_calories_and_items( player &p, items_location loc )
{
    std::pair<int, int> calories_items = std::make_pair( 0, 0 );
    const std::vector<item *> drops = find_forage_items( &p, loc );
    for( const auto &item : drops ) {
        int calories = 0;
        if( item->is_comestible() ) {
            calories = item->type->comestible->get_calories() * item->charges;
            global_contents[item->typeId()] += item->charges;
        }
        calories_items = std::make_pair( calories_items.first + calories,
                                         calories_items.second + 1 );
    }
    i_clear_adjacent( p.pos() );
    return calories_items;
}

// how many calories, items will you get from searching a hundred bushes a hundred times?
std::pair<int, int> hundred_bushes_hundred( items_location loc, int survival = 0,
        int perception = 8 )
{
    player &dummy = g->u;
    std::pair<int, int> calories_items = std::make_pair( 0, 0 );
    std::pair<int, int> calories_items_total = std::make_pair( 0, 0 );
    dummy.set_skill_level( skill_id( "survival" ), survival );
    dummy.per_cur = perception;
    for( int j = 0; j < 100; j++ ) {
        for( int i = 0; i < 100; i++ ) {
            const auto new_pair = forage_calories_and_items( dummy, "forage_spring" );
            calories_items = std::make_pair( new_pair.first + calories_items.first,
                                             new_pair.second + calories_items.second );
        }
        calories_items_total.first += calories_items.first;
        calories_items_total.second += calories_items.second;
        calories_items.first = 0;
        calories_items.second = 0;
    }
    return calories_items_total;
}

// how many bushes do you need to search to feed yourself for a day?
std::pair<int, int> how_many_bushes( items_location loc, int survival = 0, int perception = 8 )
{
    int number_of_bushes = 0;
    int number_of_calories = 0;
    int number_of_successes = 0;
    std::pair<int, int> calories_items = std::make_pair( 0, 0 );
    const int daily_calories = 2500;

    player &dummy = g->u;
    dummy.set_skill_level( skill_id( "survival" ), survival );
    dummy.per_cur = perception;

    while( number_of_calories < daily_calories ) {
        const auto new_pair = forage_calories_and_items( dummy, loc );
        number_of_calories += new_pair.first;
        number_of_successes += new_pair.second;
        number_of_bushes++;
    }
    return std::make_pair( number_of_bushes, number_of_successes );
}

// gets all harvest information from an overmap tile
std::map<harvest_id, int> get_map_terrain_harvest( const tripoint &p )
{
    std::map<harvest_id, int> harvests;
    tripoint t = p;
    for( int x = 0; x < SEEX * 2; ++x ) {
        for( int y = 0; y < SEEY * 2; ++y ) {
            t.x = p.x + x;
            t.y = p.y + y;
            harvests[ g->m.get_harvest( t ) ]++;
        }
    }
    return harvests;
}

// returns a map with names and counts of terrains
std::map<std::string, int> get_map_terrain( const tripoint &p )
{
    std::map<std::string, int> terrains;
    tripoint t = p;
    for( int x = 0; x < SEEX * 2; ++x ) {
        for( int y = 0; y < SEEY * 2; ++y ) {
            t.x = p.x + x;
            t.y = p.y + y;
            terrains[g->m.tername( t )]++;
        }
    }
    return terrains;
}

// gives a vector of tripoints with harvestable terrain or furniture
std::vector<tripoint> get_harvest_pos( const tripoint &p )
{
    std::vector<tripoint> harvests;
    tripoint t = p;
    for( int x = 0; x < SEEX * 2; ++x ) {
        for( int y = 0; y < SEEY * 2; ++y ) {
            t.x = p.x + x;
            t.y = p.y + y;
            if( g->m.get_harvest( t ) != harvest_id( "null" ) ) {
                harvests.push_back( t );
            }
        }
    }
    return harvests;
}

// gets calories from a single harvest action
int harvest_calories( const player &p, const tripoint &hv_p )
{
    std::vector<item> harvested = harvest_terrain( p, hv_p );
    int calories = 0;
    for( const item &it : harvested ) {
        if( it.is_comestible() ) {
            calories += it.type->comestible->get_calories();
            global_contents[it.typeId()] += it.charges;
        } else {
            std::string hv_it = it.typeId().c_str();
            /**
             * The numbers below are hardcoded because currently
             * the items they represent are not edible comestibles
             * directly, rather they need to be cooked over a fire.
             */
            if( hv_it == "pinecone" ) {
                global_contents[hv_it]++;
                calories += 202;
            } else if( hv_it == "chestnut" ) {
                global_contents[hv_it]++;
                calories += 288;
            } else if( hv_it == "hazelnut" ) {
                global_contents[hv_it]++;
                calories += 193;
            } else if( hv_it == "hickory_nut" ) {
                global_contents[hv_it]++;
                calories += 193;
            } else if( hv_it == "walnut" ) {
                global_contents[hv_it]++;
                calories += 193;
            } else {
                global_no_kcal.emplace( std::make_pair( hv_it, true ) );
            }
        }
    }
    return calories;
}

// gets calories for harvest actions from a vector of points
int harvest_calories( const player &p, const std::vector<tripoint> &hv_p )
{
    int calories = 0;
    for( const tripoint tp : hv_p ) {
        calories += harvest_calories( p, tp );
    }
    return calories;
}

/**
 * gives a vector of tripoints for underbrush.
 * since underbrush is not a "harvestable terrain or furniture" it needs a seperate function
 * @TODO: Calculate time spent using these tripoints
 */
std::vector<tripoint> get_underbrush_pos( const tripoint &p )
{
    std::vector<tripoint> underbrush;
    tripoint t = p;
    for( int x = 0; x < SEEX * 2; ++x ) {
        for( int y = 0; y < SEEY * 2; ++y ) {
            t.x = p.x + x;
            t.y = p.y + y;
            if( g->m.tername( t ) == "underbrush" ) {
                underbrush.push_back( t );
            }
        }
    }
    return underbrush;
}

// returns total calories you can find in one overmap tile forest_thick
int calories_in_forest( player &p, items_location loc, bool print = false )
{
    generate_forest_OMT( p.pos() );
    std::map<std::string, int> map_terrains = get_map_terrain( p.pos() );
    std::string print_string = "";
    print_string += "\n\n";
    for( const auto &pair : map_terrains ) {
        print_string += pair.first;
        print_string += ": ";
        print_string += std::to_string( pair.second );
        print_string += "\n";
    }
    print_string += "\n";

    const int underbrush = get_underbrush_pos( p.pos() ).size();
    int calories = 0;
    int successes = 0;
    for( int i = 0; i < underbrush; i++ ) {
        calories += forage_calories_and_items( p, loc ).first;
        // trash is excluded from successes for these purposes
        if( calories > 0 ) {
            successes++;
        }
    }
    calories += harvest_calories( p, get_harvest_pos( p.pos() ) );
    print_string += "Total Calories found: ";
    print_string += std::to_string( calories );
    print_string += "\n";
    if( print ) {
        printf( "%s", print_string.c_str() );
    }
    if( calories > global_winner.second ) {
        global_winner.first = print_string;
        global_winner.second = calories;
        global_winner_contents = "";
        for( const auto &pair : global_contents ) {
            global_winner_contents += pair.first;
            global_winner_contents += ": ";
            global_winner_contents += std::to_string( pair.second );
            global_winner_contents += "\n";
        }
    }
    if( calories < global_loser.second ) {
        global_loser.first = print_string;
        global_loser.second = calories;
    }
    global_contents.clear();
    return calories;
}

// returns number of forests required to reach RDAKCAL
bool run_forage_test( const int min_forests, const int max_forests, const int season = 0,
                      const int survival = 0, const int perception = 8, bool print = false )
{
    forage_test_data data;
    calendar::turn += to_turns<int>( calendar::season_length() ) * season;
    player &dummy = g->u;
    items_location loc;
    switch( season ) {
        case 0:
            loc = "forage_spring";
            break;
        case 1:
            loc = "forage_summer";
            break;
        case 2:
            loc = "forage_autumn";
            break;
        case 3:
            loc = "forage_winter";
            break;
    }
    dummy.set_skill_level( skill_id( "skill_survival" ), survival );
    dummy.per_cur = perception;
    int calories_total = 0;
    int count = 0;
    // 2500 kCal per day in CDDA
    while( calories_total < 2500 && count <= max_forests ) {
        const int temp = calories_in_forest( dummy, loc );
        data.calories.add( temp );
        calories_total += temp;
        count++;
    }
    if( print ) {
        printf( "\n" );
        printf( "%s\n", calendar::name_season( season_of_year( calendar::turn ) ) );
        printf( "Survival: %i, Perception: %i\n", dummy.get_skill_level( skill_id( "skill_survival" ) ),
                dummy.per_cur );
        printf( "Average Calories in %i Forests: %f\n", data.calories.n(), data.calories.avg() );
        printf( "Min Calories: %i, Max Calories: %i, Std Deviation: %f\n", data.calories.min(),
                data.calories.max(),
                data.calories.stddev() );
        printf( "\n" );
    }
    if( count >= min_forests && count <= max_forests ) {
        return true;
    } else {
        return false;
    }
}

TEST_CASE( "forage_spring" )
{
    forage_test_data forests;
    int count = 0;
    const int runs = 2000;
    for( int i = 0; i < runs; i++ ) {
        if( run_forage_test( 1, 1, 0, 0, 8, false ) ) {
            count++;
        }
    }
    printf( "\n\nsuccesses between 1 and 1: %i, runs: %i\n\n\n", count, runs );
}

TEST_CASE( "forage_survival_level" )
{
    printf( "\n" );
    std::pair<int, int> res;
    int surv = 0;
    int per = 1;
    unsigned long total_bushes = 0;
    unsigned long success_bushes = 0;
    int min_bushes = 2147483647;
    int max_bushes = 0;
    int times_to_run = 5000;
    for( per = 1; per <= 20; per++ ) {
        for( surv = 2; surv <= MAX_SKILL; surv++ ) {
            printf( "\nSurvival Level %i\n", surv );
            printf( "Perception Level %i\n", per );
            for( int i = 0; i < times_to_run; i++ ) {
                res = how_many_bushes( "forage_spring", surv, per );
                total_bushes += res.first;
                success_bushes += res.second;
                min_bushes = std::min( min_bushes, res.second );
                max_bushes = std::max( max_bushes, res.first );
            }
            printf( "Number of times the test was run: %i\n", times_to_run );
            printf( "Total bushes Searched: %lu\n", total_bushes );
            printf( "Average total bushes searched: %lu\n", total_bushes / times_to_run );
            printf( "Total bushes with item: %lu\n", success_bushes );
            printf( "Average total bushes searched that contained item: %lu\n", success_bushes / times_to_run );
            printf( "Minimum bushes searched: %i\n", min_bushes );
            printf( "Maximum bushes searched: %i\n", max_bushes );
            min_bushes = 2147483647;
            max_bushes = 0;
            total_bushes = 0;
            success_bushes = 0;
        }
    }
}

TEST_CASE( "generate_forest_spring1" )
{
    player &dummy = g->u;
    printf( "\n" );
    printf( "%s\n", calendar::name_season( season_of_year( calendar::turn ) ) );
    printf( "Survival: %i, Perception: %i\n", dummy.get_skill_level( skill_id( "skill_survival" ) ),
            dummy.per_cur );
    const tripoint player_pos = dummy.pos();
    printf( "x: %i, y: %i, z: %i\n", player_pos.x, player_pos.y, player_pos.z );
    printf( "\n" );
    const int calories = calories_in_forest( dummy, "forage_spring", true );
}

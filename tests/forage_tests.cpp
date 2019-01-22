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

std::pair<std::string, int> global_winner = std::make_pair( "", 0 );
std::pair<std::string, int> global_loser = std::make_pair( "", 2147483647 );
std::map<std::string, bool> global_no_kcal;
std::string global_winner_contents = "";
std::map<itype_id, int> global_contents;

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
            if( hv_it == "pinecone" ) {
                global_contents[hv_it]++;
                calories += 202; // 4 pinecones for 4 pine nuts
            } else if( hv_it == "chestnut" ) {
                global_contents[hv_it]++;
                calories += 288;
            } else if( hv_it == "hazelnut" ) {
                global_contents[hv_it]++;
                calories += 772;
            } else if( hv_it == "hickory_nut" ) {
                global_contents[hv_it]++;
                calories += 772;
            } else if( hv_it == "walnut" ) {
                global_contents[hv_it]++;
                calories += 772;
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

float std_dev( const std::vector<int> ints )
{
    long sum = 0;
    float mean = 0;
    float std_dev = 0.0;
    for( const int elem : ints ) {
        sum += elem;
    }
    mean = sum / ints.size();
    for( const int elem : ints ) {
        std_dev += pow( elem - mean, 2 );
    }
    return sqrt( std_dev / ints.size() );
}

int avg( const std::vector<int> ints )
{
    long sum = 0;
    for( const int elem : ints ) {
        sum += elem;
    }
    return sum / ints.size();
}

void run_forage_test( int season = 0, int survival = 0, int perception = 8, bool print = false )
{
    calendar::turn += to_turns<int>( calendar::season_length() ) * season;
    player &dummy = g->u;
    std::vector<int> calories;
    const int count = 1500;
    int min_calories = 2147483647;
    int max_calories = 0;
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
    for( int i = 0; i < count; i++ ) {
        const int result = calories_in_forest( dummy, loc );
        calories.push_back( result );
        min_calories = std::min( min_calories, result );
        max_calories = std::max( max_calories, result );
    }
    if( print ) {
        printf( "\n" );
        printf( "%s\n", calendar::name_season( season_of_year( calendar::turn ) ) );
        printf( "Survival: %i, Perception: %i\n", dummy.get_skill_level( skill_id( "skill_survival" ) ),
                dummy.per_cur );
        printf( "Average Calories in %i Forests: %i\n", count, avg( calories ) );
        printf( "Min Calories: %i, Max Calories: %i, Std Deviation: %f\n", min_calories, max_calories,
                std_dev( calories ) );
        printf( "\n" );
        printf( "Items without Calories:\n" );
        for( const auto &pair : global_no_kcal ) {
            printf( "%s\n", pair.first.c_str() );
        }
        printf( "\n" );
        printf( "Global Winner at %i Calories:%s", global_winner.second, global_winner.first.c_str() );
        printf( "%s", global_winner_contents.c_str() );
    }
}

TEST_CASE( "forage_spring" )
{
    std::pair<int, int> res;
    printf( "\n" );
    for( int surv = 0; surv <= MAX_SKILL; surv++ ) {
        res = how_many_bushes( "forage_spring", surv );
        printf( "Calories met at Survival %i!\nSuccess rate: %i / %i\n\n", surv, res.second, res.first );
    }
    printf( "\n\n" );
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

TEST_CASE( "generate_forest_spring" )
{
    run_forage_test( 0, 0, 8, true );
}

TEST_CASE( "generate_forest_summer" )
{
    run_forage_test( 1, 0, 8, true );
}

TEST_CASE( "generate_forest_autumn" )
{
    run_forage_test( 2, 0, 8, true );
}

TEST_CASE( "generate_forest_winter" )
{
    run_forage_test( 3, 0, 8, true );
}

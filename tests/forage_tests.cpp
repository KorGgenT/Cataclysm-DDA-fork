#include "catch/catch.hpp"
#include "game.h"
#include "player.h"
#include "activity_handlers.h"
#include "itype.h"
#include "map.h"
#include "map_helpers.h"
#include "overmapbuffer.h"

std::pair<int, int> forage_calories_and_items( player &p, items_location loc )
{
    std::pair<int, int> calories_items = std::make_pair( 0, 0 );
    const std::vector<item *> drops = find_forage_items( &p, loc );
    for( const auto &item : drops ) {
        int calories = 0;
        if( item->is_comestible() ) {
            calories = item->type->comestible->get_calories() * item->charges;
        }
        calories_items = std::make_pair( calories_items.first + calories,
                                         calories_items.second + 1 );
    }
    g->m.i_clear( p.pos() );
    drops.~vector();
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
    player *dummy_ptr = &dummy;
    dummy_ptr->set_skill_level( skill_id( "survival" ), survival );
    dummy.per_cur = perception;

    while( number_of_calories < daily_calories ) {
        const auto new_pair = forage_calories_and_items( *dummy_ptr, loc );
        number_of_calories += new_pair.first;
        number_of_successes += new_pair.second;
        number_of_bushes++;
    }
    return std::make_pair( number_of_bushes, number_of_successes );
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

TEST_CASE("generate_forest_spring") {
    player &dummy = g->u;
    printf("\n\n%s\n\n", overmap_buffer.ter(dummy.global_omt_location()).id().str());
    generate_forest_OMT(dummy.pos());
    printf("\n\n%s\n\n", overmap_buffer.ter(dummy.global_omt_location()).id().str());
}

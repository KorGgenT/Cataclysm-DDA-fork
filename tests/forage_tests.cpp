#include "catch/catch.hpp"
#include "game.h"
#include "player.h"
#include "activity_handlers.h"
#include "itype.h"
#include "map.h"

std::pair<int, int> forage_calories_and_items( player &p, items_location loc )
{
    std::pair<int, int> calories_items = std::make_pair( 0, 0 );
    const std::vector<item *> drops = find_forage_items( &p, loc );
    for( const auto &item : drops ) {
        int calories = 0;
        if( item->is_comestible() ) {
            calories = item->type->comestible->get_calories();
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
    player *dummy_ptr = &dummy;
    std::pair<int, int> calories_items = std::make_pair( 0, 0 );
    std::pair<int, int> calories_items_total = std::make_pair( 0, 0 );
    dummy_ptr->set_skill_level( skill_id( "survival" ), survival );
    dummy.per_cur = perception;
    for( int j = 0; j < 100; j++ ) {
        for( int i = 0; i < 100; i++ ) {
            const auto new_pair = forage_calories_and_items( *dummy_ptr, "forage_spring" );
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
/*
TEST_CASE( "forage_spring" )
{
    std::pair<int, int> res;
    printf( "\n" );
    for( int surv = MAX_SKILL; surv >= 0 ; surv-- ) {
        int ratio;
        for( int i = 0; i < 10; i++ ) {
            res = hundred_bushes_hundred( "forage_spring", surv );
            ratio = res.first / res.second;
            if( surv > 3 ) {
                int min_ratio = 28 + surv;
                if (surv <= 5) {
                    min_ratio--;
                }
                REQUIRE( ratio >= min_ratio );
            }
            int max_ratio = 36 + surv;
            if (surv < 8) {
                max_ratio--;
            }
            REQUIRE( ratio <= max_ratio );
        }
        printf( "Survival level %i ratio success\n", surv );
    }
}
*/
TEST_CASE( "forage_survival_level" )
{
    printf( "\n" );
    std::pair<int, int> res;
    const int surv = 8;
    for( int i = 0; i < 100; i++ ) {
        res = hundred_bushes_hundred( "forage_spring", surv );
        printf( "\n100 bushes, 100 runs. Perception 8, Spring.\nSurvival: %i, Total Calories: %i, Total Successes: %i, Ratio: %i\n",
                surv, res.first, res.second, res.first / res.second );
    }
}

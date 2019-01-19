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
    return calories_items;
}

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

TEST_CASE( "forage_spring" )
{
    std::pair<int, int> res;
    printf( "\n" );
    for( int surv = MAX_SKILL; surv >= 0 ; surv-- ) {
        res = hundred_bushes_hundred( "forage_spring", surv );
        printf( "\n100 bushes, 100 runs. Perception 8, Spring.\nSurvival: %i, Total Calories: %i, Total Successes: %i\n",
                surv, res.first, res.second );
    }
    printf("\n");
    res = hundred_bushes_hundred("forage_spring", 2);
    printf("\n100 bushes, 100 runs. Perception 8, Spring.\nSurvival: %i, Total Calories: %i, Total Successes: %i\n",
        2, res.first, res.second);
    printf("\n");
    res = hundred_bushes_hundred("forage_spring", 6);
    printf("\n100 bushes, 100 runs. Perception 8, Spring.\nSurvival: %i, Total Calories: %i, Total Successes: %i\n",
        6, res.first, res.second);
}

TEST_CASE("forage_survival9") {
    printf("\n");
    const std::pair<int, int> res = hundred_bushes_hundred("forage_spring", 9);
    printf("\n100 bushes, 100 runs. Perception 8, Spring.\nSurvival: %i, Total Calories: %i, Total Successes: %i\n",
        9, res.first, res.second);
}

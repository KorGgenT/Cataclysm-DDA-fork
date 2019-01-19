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
                                         calories_items.second + item->charges );
    }
}

TEST_CASE( "forage_spring" )
{
    player &dummy = g->u;
    std::pair<int, int> calories_items = std::make_pair( 0, 0 );
    for( int i = 0; i < 100; i++ ) {
        const auto new_pair = forage_calories_and_items( &dummy, "forage_spring" );
        calories_items = std::make_pair( new_pair.first + calories_items.first,
                                         new_pair.second + calories_items.second );
    }
    printf("calories: %i, number of items: %i", calories_items.first, calories_items.second);
}

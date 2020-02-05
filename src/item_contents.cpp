#include "item_contents.h"

#include "generic_factory.h"
#include "item.h"
#include "item_pocket.h"
#include "optional.h"
#include "point.h"
#include "units.h"

namespace fake_item
{
static item_pocket none_pocket( nullptr );
static pocket_data legacy_pocket_data;
static item_pocket legacy_pocket( &legacy_pocket_data );
} // namespace fake_item

void item_contents::add_legacy_pocket()
{
    for( const item_pocket &pocket : contents ) {
        if( pocket.is_type( item_pocket::pocket_type::LEGACY_CONTAINER ) ) {
            // an item should not have more than one legacy pocket
            return;
        }
    }
    contents.emplace_front( fake_item::legacy_pocket );
}

bool item_contents::stacks_with( const item_contents &rhs ) const
{
    if( contents.size() != rhs.contents.size() ) {
        return false;
    }
    return std::equal( contents.begin(), contents.end(),
                       rhs.contents.begin(),
    []( const item_pocket & a, const item_pocket & b ) {
        return a.stacks_with( b );
    } );
}

bool item_contents::same_contents( const item_contents &rhs ) const
{
    if( contents.size() != rhs.contents.size() ) {
        return false;
    }
    return std::equal( contents.begin(), contents.end(),
                       rhs.contents.begin(), rhs.contents.end(),
    []( const item_pocket & a, const item_pocket & b ) {
        return a.same_contents( b );
    } );
}

item_pocket *item_contents::best_pocket( const item &it, bool nested )
{
    if( !can_contain( it ).success() ) {
        return nullptr;
    }
    item_pocket *ret = nullptr;
    for( item_pocket &pocket : contents ) {
        if( nested && !pocket.rigid() ) {
            continue;
        }
        if( ret == nullptr ) {
            if( pocket.can_contain( it ).success() ) {
                ret = &pocket;
            }
        } else if( pocket.can_contain( it ).success() && ret->better_pocket( pocket, it ) ) {
            ret = &pocket;
            for( item *contained : all_items_top() ) {
                item_pocket *internal_pocket = contained->contents.best_pocket( it, nested );
                if( internal_pocket != nullptr && ret->better_pocket( pocket, it ) ) {
                    ret = internal_pocket;
                }
            }
        }
    }
    return ret;
}

void item_contents::combine( const item_contents &rhs )
{
    for( const item_pocket &pocket : rhs.contents ) {
        if( pocket.saved_type() == item_pocket::pocket_type::LEGACY_CONTAINER ) {
            for( const item *it : pocket.all_items_top() ) {
                insert_legacy( *it );
            }
        } else {
            for( const item *it : pocket.all_items_top() ) {
                const ret_val<bool> inserted = insert_item( *it, pocket.saved_type() );
                if( !inserted.success() ) {
                    debugmsg( "error: tried to put an item into a pocket that can't fit it while loading. err: %s",
                              inserted.str() );
                }
            }
        }
    }
}

void item_contents::move_legacy_to_pocket_type( const item_pocket::pocket_type pk_type )
{
    for( const item &it : legacy_pocket().all_items() ) {
        if( !insert_item( it, pk_type ).success() ) {
            force_insert_item( it, pk_type );
        }
    }
    legacy_pocket().clear_items();
}

ret_val<bool> item_contents::can_contain_rigid( const item &it ) const
{
    ret_val<bool> ret = ret_val<bool>::make_failure( _( "is not a container" ) );
    for( const item_pocket &pocket : contents ) {
        if( !pocket.rigid() ) {
            ret = ret_val<bool>::make_failure( _( "is not rigid" ) );
            continue;
        }
        const ret_val<item_pocket::contain_code> pocket_contain_code = pocket.can_contain( it );
        if( pocket_contain_code.success() ) {
            return ret_val<bool>::make_success();
        }
        if( pocket_contain_code.value() != item_pocket::contain_code::ERR_LEGACY_CONTAINER ) {
            ret = ret_val<bool>::make_failure( pocket_contain_code.str() );
        }
    }
    return ret;
}

ret_val<bool> item_contents::can_contain( const item &it ) const
{
    ret_val<bool> ret = ret_val<bool>::make_failure( _( "is not a container" ) );
    for( const item_pocket &pocket : contents ) {
        const ret_val<item_pocket::contain_code> pocket_contain_code = pocket.can_contain( it );
        if( pocket_contain_code.success() ) {
            return ret_val<bool>::make_success();
        }
        if( pocket_contain_code.value() != item_pocket::contain_code::ERR_LEGACY_CONTAINER ) {
            ret = ret_val<bool>::make_failure( pocket_contain_code.str() );
        }
    }
    return ret;
}

item *item_contents::magazine_current()
{
    for( item_pocket &pocket : contents ) {
        item *mag = pocket.magazine_current();
        if( mag != nullptr ) {
            return mag;
        }
    }
    return nullptr;
}

void item_contents::casings_handle( const std::function<bool( item & )> &func )
{
    for( item_pocket &pocket : contents ) {
        pocket.casings_handle( func );
    }
}

bool item_contents::use_amount( const itype_id &it, int &quantity, std::list<item> &used )
{
    bool used_item = false;
    for( item_pocket &pocket : contents ) {
        used_item = pocket.use_amount( it, quantity, used ) || used_item;
    }
    return used_item;
}

bool item_contents::will_explode_in_a_fire() const
{
    for( const item_pocket &pocket : contents ) {
        if( pocket.will_explode_in_a_fire() ) {
            return true;
        }
    }
    return false;
}

bool item_contents::detonate( const tripoint &p, std::vector<item> &drops )
{
    bool detonated = false;
    for( item_pocket &pocket : contents ) {
        detonated = pocket.detonate( p, drops ) || detonated;
    }
    return detonated;
}

bool item_contents::process( const itype &type, player *carrier, const tripoint &pos,
                             bool activate,
                             float insulation, const temperature_flag flag )
{
    for( item_pocket &pocket : contents ) {
        pocket.process( type, carrier, pos, activate, insulation, flag );
    }
    return true;
}

bool item_contents::legacy_unload( player &guy, bool &changed )
{
    for( item_pocket &pocket : contents ) {
        pocket.legacy_unload( guy, changed );
    }
    return true;
}

void item_contents::remove_all_ammo( Character &guy )
{
    for( item_pocket &pocket : contents ) {
        pocket.remove_all_ammo( guy );
    }
}

void item_contents::remove_all_mods( Character &guy )
{
    for( item_pocket &pocket : contents ) {
        pocket.remove_all_mods( guy );
    }
}

bool item_contents::legacy_empty() const
{
    return legacy_pocket().empty();
}

bool item_contents::empty() const
{
    if( contents.empty() ) {
        return true;
    }
    for( const item_pocket &pocket : contents ) {
        if( !pocket.empty() ) {
            return false;
        }
    }
    return true;
}

item &item_contents::legacy_back()
{
    return legacy_pocket().back();
}

const item &item_contents::legacy_back() const
{
    return legacy_pocket().back();
}

item &item_contents::legacy_front()
{
    return legacy_pocket().front();
}

const item &item_contents::legacy_front() const
{
    return legacy_pocket().front();
}

size_t item_contents::legacy_size() const
{
    if( contents.empty() ) {
        return 0;
    }
    return legacy_pocket().size();
}

void item_contents::legacy_pop_back()
{
    legacy_pocket().pop_back();
}

size_t item_contents::num_item_stacks() const
{
    size_t num = 0;
    for( const item_pocket &pocket : contents ) {
        num += pocket.size();
    }
    return num;
}

size_t item_contents::size() const
{
    // always has a legacy pocket due to contstructor
    // we want to ignore the legacy pocket
    return contents.size() - 1;
}

item_pocket &item_contents::legacy_pocket()
{
    for( item_pocket &pocket : contents ) {
        if( pocket.is_type( item_pocket::pocket_type::LEGACY_CONTAINER ) ) {
            return pocket;
        }
    }
    debugmsg( "Tried to access non-existing legacy pocket" );
    return fake_item::none_pocket;
}

const item_pocket &item_contents::legacy_pocket() const
{
    for( const item_pocket &pocket : contents ) {
        if( pocket.is_type( item_pocket::pocket_type::LEGACY_CONTAINER ) ) {
            return pocket;
        }
    }
    debugmsg( "Tried to access non-existing legacy pocket" );
    return fake_item::none_pocket;
}

std::list<item> item_contents::all_items()
{
    std::list<item> item_list;
    for( item_pocket &pocket : contents ) {
        std::list<item> contained_items = pocket.all_items();
        item_list.insert( item_list.end(), contained_items.begin(), contained_items.end() );
    }
    return item_list;
}

std::list<item> item_contents::all_items() const
{
    std::list<item> item_list;
    for( const item_pocket &pocket : contents ) {
        std::list<item> contained_items = pocket.all_items();
        item_list.insert( item_list.end(), contained_items.begin(), contained_items.end() );
    }
    return item_list;
}

std::list<item *> item_contents::all_items_ptr( item_pocket::pocket_type pk_type )
{
    std::list<item *> all_items_internal;
    for( item_pocket &pocket : contents ) {
        if( pocket.is_type( pk_type ) ) {
            std::list<item *> contained_items = pocket.all_items_ptr( pk_type );
            all_items_internal.insert( all_items_internal.end(), contained_items.begin(),
                                       contained_items.end() );
        }
    }
    return all_items_internal;
}

std::list<const item *> item_contents::all_items_ptr( item_pocket::pocket_type pk_type ) const
{
    std::list<const item *> all_items_internal;
    for( const item_pocket &pocket : contents ) {
        if( pocket.is_type( pk_type ) ) {
            std::list<const item *> contained_items = pocket.all_items_ptr( pk_type );
            all_items_internal.insert( all_items_internal.end(), contained_items.begin(),
                                       contained_items.end() );
        }
    }
    return all_items_internal;
}

std::list<item *> item_contents::all_items_ptr()
{
    std::list<item *> all_items_internal;
    for( item_pocket &pocket : contents ) {
        for( int i = 0; i < item_pocket::pocket_type::LAST; i++ ) {
            std::list<item *> pocket_items = pocket.all_items_ptr( static_cast<item_pocket::pocket_type>( i ) );
            all_items_internal.insert( all_items_internal.end(), pocket_items.begin(), pocket_items.end() );
        }
    }
    return all_items_internal;
}

std::list<const item *> item_contents::all_items_ptr() const
{
    std::list<const item *> all_items_internal;
    for( const item_pocket &pocket : contents ) {
        for( int i = 0; i < item_pocket::pocket_type::LAST; i++ ) {
            std::list<const item *> pocket_items = pocket.all_items_ptr( static_cast<item_pocket::pocket_type>
                                                   ( i ) );
            all_items_internal.insert( all_items_internal.end(), pocket_items.begin(), pocket_items.end() );
        }
    }
    return all_items_internal;
}

std::list<item *> item_contents::all_items_top( item_pocket::pocket_type pk_type )
{
    std::list<item *> all_items_internal;
    for( item_pocket &pocket : contents ) {
        if( pocket.is_type( pk_type ) ) {
            std::list<item *> contained_items = pocket.all_items_top();
            all_items_internal.insert( all_items_internal.end(), contained_items.begin(),
                                       contained_items.end() );
        }
    }
    return all_items_internal;
}

units::volume item_contents::item_size_modifier() const
{
    units::volume total_vol = 0_ml;
    for( const item_pocket &pocket : contents ) {
        if( !pocket.is_type( item_pocket::pocket_type::LEGACY_CONTAINER ) ) {
            total_vol += pocket.item_size_modifier();
        }
    }
    return total_vol;
}

units::volume item_contents::total_container_capacity() const
{
    units::volume total_vol = 0_ml;
    for( const item_pocket &pocket : contents ) {
        if( pocket.is_type( item_pocket::pocket_type::CONTAINER ) ) {
            total_vol += pocket.volume_capacity();
        }
    }
    return total_vol;
}

units::volume item_contents::remaining_container_capacity() const
{
    units::volume total_vol = 0_ml;
    for( const item_pocket &pocket : contents ) {
        if( pocket.is_type( item_pocket::pocket_type::CONTAINER ) ) {
            total_vol += pocket.remaining_volume();
        }
    }
    return total_vol;
}

units::volume item_contents::remaining_liquid_capacity( const item &liquid ) const
{
    units::volume total_vol = 0_ml;
    for( const item_pocket &pocket : contents ) {
        if( pocket.is_type( item_pocket::pocket_type::CONTAINER ) && pocket.watertight()
            && ( pocket.size() == 0 || ( pocket.size() == 1 && pocket.has_item_stacks_with( liquid ) ) ) ) {
            total_vol += pocket.remaining_volume();
        }
    }
    return total_vol;
}

units::volume item_contents::total_contained_volume() const
{
    units::volume total_vol = 0_ml;
    for( const item_pocket &pocket : contents ) {
        if( pocket.is_type( item_pocket::pocket_type::CONTAINER ) ) {
            total_vol += pocket.contains_volume();
        }
    }
    return total_vol;
}

units::mass item_contents::item_weight_modifier() const
{
    units::mass total_mass = 0_gram;
    for( const item_pocket &pocket : contents ) {
        total_mass += pocket.item_weight_modifier();
    }
    return total_mass;
}

void item_contents::overflow( const tripoint &pos )
{
    for( item_pocket &pocket : contents ) {
        pocket.overflow( pos );
    }
}

bool item_contents::spill_contents( const tripoint &pos )
{
    for( item_pocket &pocket : contents ) {
        pocket.spill_contents( pos );
    }
    return true;
}

cata::optional<item> item_contents::remove_item( const item &it )
{
    for( item_pocket &pocket : contents ) {
        cata::optional<item> ret = pocket.remove_item( it );
        if( ret ) {
            return ret;
        }
    }
    return cata::nullopt;
}

cata::optional<item> item_contents::remove_item( const item_location &it )
{
    if( !it ) {
        return cata::nullopt;
    }
    return remove_item( *it );
}

void item_contents::remove_internal( const std::function<bool( item & )> &filter,
                                     int &count, std::list<item> &res )
{
    for( item_pocket &pocket : contents ) {
        if( pocket.remove_internal( filter, count, res ) ) {
            return;
        }
    }
}

void item_contents::clear_items()
{
    for( item_pocket &pocket : contents ) {
        pocket.clear_items();
    }
}

bool item_contents::has_item( const item &it ) const
{
    for( const item_pocket &pocket : contents ) {
        if( pocket.has_item( it ) ) {
            return true;
        }
    }
    return false;
}

item *item_contents::get_item_with( const std::function<bool( const item & )> &filter )
{
    for( item_pocket &pocket : contents ) {
        item *it = pocket.get_item_with( filter );
        if( it != nullptr ) {
            return it;
        }
    }
    return nullptr;
}

void item_contents::remove_items_if( const std::function<bool( item & )> &filter )
{
    for( item_pocket &pocket : contents ) {
        pocket.remove_items_if( filter );
    }
}

void item_contents::has_rotten_away( const tripoint &pnt )
{
    for( item_pocket &pocket : contents ) {
        pocket.has_rotten_away( pnt );
    }
}

ret_val<item_pocket *> item_contents::find_pocket_for( const item &it,
        item_pocket::pocket_type pk_type )
{
    static item_pocket *null_pocket = nullptr;
    ret_val<item_pocket *> ret = ret_val<item_pocket *>::make_failure( null_pocket,
                                 _( "is not a container" ) );
    for( item_pocket &pocket : contents ) {
        if( !pocket.is_type( pk_type ) ) {
            continue;
        }
        ret_val<item_pocket::contain_code> ret_contain = pocket.can_contain( it );
        if( ret_contain.success() ) {
            return ret_val<item_pocket *>::make_success( &pocket, ret_contain.str() );
        }
    }
    return ret;
}

int item_contents::insert_cost( const item &it, item_pocket::pocket_type pk_type )
{
    ret_val<item_pocket *> pocket = find_pocket_for( it, pk_type );
    if( pocket.success() ) {
        return pocket.value()->moves();
    } else {
        return -1;
    }
}

void item_contents::force_insert_item( const item &it, item_pocket::pocket_type pk_type )
{
    for( item_pocket &pocket : contents ) {
        if( pocket.is_type( pk_type ) ) {
            pocket.add( it );
            return;
        }
    }
    debugmsg( "ERROR: Could not insert item %s as contents does not have pocket type", it.tname() );
}

ret_val<bool> item_contents::insert_item( const item &it, item_pocket::pocket_type pk_type )
{
    ret_val<item_pocket *> pocket = find_pocket_for( it, pk_type );
    if( !pocket.success() ) {
        return ret_val<bool>::make_failure( pocket.str() );
    }
    ret_val<item_pocket::contain_code> pocket_contain_code = pocket.value()->insert_item( it );
    if( pocket_contain_code.success() ) {
        return ret_val<bool>::make_success();
    }
    if( pocket_contain_code.value() != item_pocket::contain_code::ERR_LEGACY_CONTAINER ) {
        return ret_val<bool>::make_failure( pocket_contain_code.str() );
    }
    return ret_val<bool>::make_failure( "No success" );
}

int item_contents::obtain_cost( const item &it ) const
{
    for( const item_pocket &pocket : contents ) {
        const int mv = pocket.obtain_cost( it );
        if( mv != 0 ) {
            return mv;
        }
    }
    return 0;
}

static void insert_separation_line( std::vector<iteminfo> &info )
{
    if( info.empty() || info.back().sName != "--" ) {
        info.push_back( iteminfo( "DESCRIPTION", "--" ) );
    }
}

void item_contents::info( std::vector<iteminfo> &info ) const
{
    int pocket_number = 1;
    std::vector<iteminfo> contents_info;
    std::vector<item_pocket> found_pockets;
    std::map<int, int> pocket_num; // index, amount
    for( const item_pocket &pocket : contents ) {
        if( !pocket.is_type( item_pocket::pocket_type::LEGACY_CONTAINER ) ) {
            bool found = false;
            int idx = 0;
            for( const item_pocket &found_pocket : found_pockets ) {
                if( found_pocket == pocket ) {
                    found = true;
                    pocket_num[idx]++;
                }
                idx++;
            }
            if( !found ) {
                found_pockets.push_back( pocket );
                pocket_num[idx]++;
            }
            pocket.contents_info( contents_info, pocket_number++, size() != 1 );
        }
    }
    int idx = 0;
    for( const item_pocket &pocket : found_pockets ) {
        insert_separation_line( info );
        if( pocket_num[idx] > 1 ) {
            info.emplace_back( "DESCRIPTION", _( string_format( "<bold>Pockets (%d)</bold>",
                                                 pocket_num[idx] ) ) );
        }
        idx++;
        pocket.general_info( info, idx, false );
    }
    info.insert( info.end(), contents_info.begin(), contents_info.end() );
}

void item_contents::insert_legacy( const item &it )
{
    legacy_pocket().add( it );
}

std::list<item> &item_contents::legacy_items()
{
    return legacy_pocket().edit_contents();
}

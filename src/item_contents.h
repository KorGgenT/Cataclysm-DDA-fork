#pragma once
#ifndef ITEM_CONTENTS_H
#define ITEM_CONTENTS_H

#include "enums.h"
#include "item_pocket.h"
#include "optional.h"
#include "ret_val.h"
#include "units.h"
#include "visitable.h"

class item;
class item_location;
class player;

struct iteminfo;
struct tripoint;

class item_contents
{
    public:
        item_contents() {
            // items should have a legacy pocket until everything is migrated
            add_legacy_pocket();
        }

        // used for loading itype
        item_contents( const std::vector<pocket_data> &pockets ) {
            for( const pocket_data &data : pockets ) {
                contents.push_back( item_pocket( &data ) );
            }
            add_legacy_pocket();
        }

        // for usage with loading to aid migration
        void add_legacy_pocket();

        bool stacks_with( const item_contents &rhs ) const;

        ret_val<bool> can_contain_rigid( const item &it ) const;
        ret_val<bool> can_contain( const item &it ) const;
        bool empty() const;
        bool legacy_empty() const;

        // all the items contained in each pocket combined into one list
        std::list<item> all_items();
        std::list<item> all_items() const;
        // all item pointers in a specific pocket type
        // used for inventory screen
        std::list<item *> all_items_ptr( item_pocket::pocket_type pk_type );
        std::list<const item *> all_items_ptr( item_pocket::pocket_type pk_type ) const;
        std::list<item *> all_items_ptr();
        std::list<const item *> all_items_ptr() const;

        std::list<item *> all_items_top( item_pocket::pocket_type pk_type =
                                             item_pocket::pocket_type::CONTAINER );

        // total size the parent item needs to be modified based on rigidity of pockets
        units::volume item_size_modifier() const;
        units::volume total_container_capacity() const;
        units::volume total_contained_volume() const;
        units::volume remaining_container_capacity() const;
        units::volume remaining_liquid_capacity( const item &liquid ) const;
        // total weight the parent item needs to be modified based on weight modifiers of pockets
        units::mass item_weight_modifier() const;

        item *magazine_current();
        void casings_handle( const std::function<bool( item & )> &func );
        bool use_amount( const itype_id &it, int &quantity, std::list<item> &used );
        bool will_explode_in_a_fire() const;
        bool detonate( const tripoint &p, std::vector<item> &drops );
        bool process( const itype &type, player *carrier, const tripoint &pos, bool activate,
                      float insulation, temperature_flag flag );
        bool legacy_unload( player &guy, bool &changed );
        void remove_all_ammo( Character &guy );
        void remove_all_mods( Character &guy );

        // removes and returns the item from the pocket.
        cata::optional<item> remove_item( const item &it );
        cata::optional<item> remove_item( const item_location &it );

        // attempts to put the input contents into the item_contents
        // copies the content items
        // used to help with loading
        void combine( const item_contents &rhs );
        // moves all items from the legacy pocket to new pocket types using can_contain
        void move_legacy_to_pocket_type( const item_pocket::pocket_type pk_type );
        void force_insert_item( const item &it, item_pocket::pocket_type pk_type );
        // tries to put an item in a pocket. returns false on failure
        // has similar code to can_contain in order to avoid running it twice
        ret_val<bool> insert_item( const item &it,
                                   item_pocket::pocket_type pk_type = item_pocket::pocket_type::CONTAINER );
        int insert_cost( const item &it,
                         item_pocket::pocket_type pk_type = item_pocket::pocket_type::CONTAINER );
        // finds or makes a fake pocket and puts this item into it
        void insert_legacy( const item &it );
        // equivalent to contents.back() when item::contents was a std::list<item>
        std::list<item> &legacy_items();
        item &legacy_back();
        const item &legacy_back() const;
        item &legacy_front();
        const item &legacy_front() const;
        size_t legacy_size() const;
        // ignores legacy_pocket, so -1
        size_t size() const;
        void legacy_pop_back();
        size_t num_item_stacks() const;
        // spills items that can't fit in the pockets anymore
        void overflow( const tripoint &pos );
        bool spill_contents( const tripoint &pos );
        void clear_items();
        bool has_item( const item &it ) const;
        item *get_item_with( const std::function<bool( const item & )> &filter );
        void remove_items_if( const std::function<bool( item & )> &filter );
        void has_rotten_away( const tripoint &pnt );

        int obtain_cost( const item &it ) const;

        void remove_internal( const std::function<bool( item & )> &filter,
                              int &count, std::list<item> &res );
        // @relates visitable
        // NOTE: upon expansion, this may need to be filtered by type enum depending on accessibility
        VisitResponse visit_contents( const std::function<VisitResponse( item *, item * )> &func,
                                      item *parent = nullptr );

        void info( std::vector<iteminfo> &info ) const;

        bool same_contents( const item_contents &rhs ) const;
        // finds the best pocket in the contents.
        // searches through internal items as well
        // if @nested then searches through pockets that
        // are relevant if they are on an item that is contained
        item_pocket *best_pocket( const item &it, bool nested = false );

        void serialize( JsonOut &json ) const;
        void deserialize( JsonIn &jsin );
    private:
        // finds the pocket the item will fit in, given the pocket type.
        // this will be where the algorithm picks the best pocket in the contents
        // returns nullptr if none is found
        ret_val<item_pocket *> find_pocket_for( const item &it,
                                                item_pocket::pocket_type pk_type = item_pocket::pocket_type::CONTAINER );
        // gets the pocket described as legacy, or creates one
        item_pocket &legacy_pocket();
        const item_pocket &legacy_pocket() const;

        std::list<item_pocket> contents;
};

#endif

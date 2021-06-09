#pragma once
#ifndef CATA_SRC_MAPGEN_FACTORY_H
#define CATA_SRC_MAPGEN_FACTORY_H

#include "generic_factory.h"
#include "map_extras.h"
#include "mapgen.h"

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
///// mapgen_function class.
///// all sorts of ways to apply our hellish reality to a grid-o-squares

class mapgen_basic_container
{
    private:
        std::vector<std::shared_ptr<mapgen_function>> mapgens_;
        weighted_int_list<std::shared_ptr<mapgen_function>> weights_;

    public:
        int add( const std::shared_ptr<mapgen_function> &ptr ) {
            cata_assert( ptr );
            mapgens_.push_back( ptr );
            return mapgens_.size() - 1;
        }
        /**
         * Pick a mapgen function randomly and call its generate function.
         * This basically runs the mapgen functions with the given @ref mapgendata
         * as argument.
         * @return Whether the mapgen function has been run. It may not get run if
         * the list of mapgen functions is effectively empty.
         * @p hardcoded_weight Weight for an additional entry. If that entry is chosen,
         * false is returned. If unsure, just use 0 for it.
         */
        bool generate( mapgendata &dat, const int hardcoded_weight ) const {
            if( hardcoded_weight > 0 &&
                rng( 1, weights_.get_weight() + hardcoded_weight ) > weights_.get_weight() ) {
                return false;
            }
            const std::shared_ptr<mapgen_function> *const ptr = weights_.pick();
            if( !ptr ) {
                return false;
            }
            cata_assert( *ptr );
            ( *ptr )->generate( dat );
            return true;
        }
        /**
         * Calls @ref mapgen_function::setup and sets up the internal weighted list using
         * the **current** value of @ref mapgen_function::weight. This value may have
         * changed since it was first added, so this is needed to recalculate the weighted list.
         */
        void setup() {
            for( const std::shared_ptr<mapgen_function> &ptr : mapgens_ ) {
                const int weight = ptr->weight;
                if( weight < 1 ) {
                    continue; // rejected!
                }
                weights_.add( ptr, weight );
                ptr->setup();
            }
            // Not needed anymore, pointers are now stored in weights_ (or not used at all)
            mapgens_.clear();
        }
        void check_consistency() {
            for( auto &mapgen_function_ptr : weights_ ) {
                mapgen_function_ptr.obj->check();
            }
        }
};

class mapgen_factory
{
    private:
        std::map<std::string, mapgen_basic_container> mapgens_;

        /// Collect all the possible and expected keys that may get used with @ref pick.
        static std::set<std::string> get_usages() {
            std::set<std::string> result;
            for( const oter_t &elem : overmap_terrains::get_all() ) {
                result.insert( elem.get_mapgen_id() );
                result.insert( elem.id.str() );
            }
            // Why do I have to repeat the MapExtras here? Wouldn't "MapExtras::factory" be enough?
            for( const map_extra &elem : MapExtras::mapExtraFactory().get_all() ) {
                if( elem.generator_method == map_extra_method::mapgen ) {
                    result.insert( elem.generator_id );
                }
            }
            // Used in C++ code only, see calls to `oter_mapgen.generate()` below
            result.insert( "lab_1side" );
            result.insert( "lab_4side" );
            result.insert( "lab_finale_1level" );
            return result;
        }

    public:
        void reset() {
            mapgens_.clear();
        }
        /// @see mapgen_basic_container::setup
        void setup() {
            for( std::pair<const std::string, mapgen_basic_container> &omw : mapgens_ ) {
                omw.second.setup();
            }
            // Dummy entry, overmap terrain null should never appear and is therefore never generated.
            mapgens_.erase( "null" );
        }
        void check_consistency() {
            // Cache all strings that may get looked up here so we don't have to go through
            // all the sources for them upon each loop.
            const std::set<std::string> usages = get_usages();
            for( std::pair<const std::string, mapgen_basic_container> &omw : mapgens_ ) {
                omw.second.check_consistency();
                if( usages.count( omw.first ) == 0 ) {
                    debugmsg( "Mapgen %s is not used by anything!", omw.first );
                }
            }
        }
        /**
         * Checks whether we have an entry for the given key.
         * Note that the entry itself may not contain any valid mapgen instance
         * (could all have been removed via @ref erase).
         */
        bool has( const std::string &key ) const {
            return mapgens_.count( key ) != 0;
        }
        /// @see mapgen_basic_container::add
        int add( const std::string &key, const std::shared_ptr<mapgen_function> &ptr ) {
            return mapgens_[key].add( ptr );
        }
        /// @see mapgen_basic_container::generate
        bool generate( mapgendata &dat, const std::string &key, const int hardcoded_weight = 0 ) const {
            const auto iter = mapgens_.find( key );
            if( iter == mapgens_.end() ) {
                return false;
            }
            return iter->second.generate( dat, hardcoded_weight );
        }
};

static mapgen_factory oter_mapgen;

#endif

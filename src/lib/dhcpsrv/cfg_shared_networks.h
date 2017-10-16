// Copyright (C) 2017 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef CFG_SHARED_NETWORKS_H
#define CFG_SHARED_NETWORKS_H

#include <cc/cfg_to_element.h>
#include <cc/data.h>
#include <exceptions/exceptions.h>
#include <dhcpsrv/shared_network.h>
#include <boost/shared_ptr.hpp>
#include <string>

namespace isc {
namespace dhcp {

/// @brief This class holds configuration of shared networks.
///
/// This is a generic class implementing basic functions such as shared network
/// addition, removal and retrieval. It also dumps configuration in the JSON
/// format.
///
/// There are specializations of this class implemented as
/// @ref CfgSharedNetworks4 and @ref CfgSharedNetworks6 for IPv4 and IPv6 cases
/// repspectively.
///
/// @tparam Type of the pointer to a shared network, i.e. @ref SharedNetwork4Ptr
/// or @ref SharedNetwork6Ptr.
template<typename SharedNetworkPtrType, typename SharedNetworkCollection>
class CfgSharedNetworks : public data::CfgToElement {
public:

    /// @brief Adds new shared network to the configuration.
    ///
    /// @param network Pointer to a network
    ///
    /// @throw isc::BadValue when name is a duplicate of existing network's
    /// name.
    void add(const SharedNetworkPtrType& network) {
        if (getByName(network->getName())) {
            isc_throw(BadValue, "duplicate network '" << network->getName() <<
                      "' found in the configuration");
        }

        networks_.push_back(network);
    }

    /// @brief Deletes shared network from the configuration.
    ///
    /// @param name Name of the network to be deleted.
    ///
    /// @throw isc::BadValue if the network can't be found.
    void del(const std::string& name) {
        auto& index = networks_.template get<SharedNetworkNameIndexTag>();
        auto shared_network = index.find(name);
        if (shared_network != index.end()) {
            index.erase(shared_network);

        } else {
            isc_throw(BadValue, "unable to delete non-existing network '"
                      << name << "' from shared networks configuration");
        }
    }

    /// @brief Retrieves shared network by name.
    ///
    /// @param name Name of the network to be retrieved.
    ///
    /// @return Pointer to the shared network or null pointer if the network
    /// is not found.
    SharedNetworkPtrType getByName(const std::string& name) const {
        const auto& index = networks_.template get<SharedNetworkNameIndexTag>();
        auto shared_network = index.find(name);
        if (shared_network != index.cend()) {
            return (*shared_network);
        }
        return (SharedNetworkPtrType());
    }

    /// @brief Unparses shared networks configuration.
    ///
    /// @return Element object representing a list of shared networks held
    /// within configuration. The networks are sorted by their names.
    virtual data::ElementPtr toElement() const {
        data::ElementPtr list = data::Element::createList();

        // Insert shared networks sorted by their names into the list.
        const auto& index = networks_.template get<SharedNetworkNameIndexTag>();
        for (auto shared_network = index.begin(); shared_network != index.end();
             ++shared_network) {
            list->add((*shared_network)->toElement());
        }
        return (list);
    }

protected:

    /// @brief Multi index container holding shared networks.
    SharedNetworkCollection networks_;
};

/// @brief Represents configuration of IPv4 shared networks.
class CfgSharedNetworks4 : public CfgSharedNetworks<SharedNetwork4Ptr,
                                                    SharedNetwork4Collection> {
public:

    /// @brief Returns pointer to all configured shared networks.
    const SharedNetwork4Collection* getAll() const {
        return (&networks_);
    }

};

/// @brief Pointer to the configuration of IPv4 shared networks.
typedef boost::shared_ptr<CfgSharedNetworks4> CfgSharedNetworks4Ptr;

/// @brief Represents configuration of IPv6 shared networks.
class CfgSharedNetworks6 : public CfgSharedNetworks<SharedNetwork6Ptr,
                                                    SharedNetwork6Collection> {
public:

    /// @brief Returns pointer to all configured shared networks.
    const SharedNetwork6Collection* getAll() const {
        return (&networks_);
    }
};

/// @brief Pointer to the configuration of IPv6 shared networks.
typedef boost::shared_ptr<CfgSharedNetworks6> CfgSharedNetworks6Ptr;


} // end of namespace isc::dhcp
} // end of namespace isc

#endif // CFG_SHARED_NETWORKS_H

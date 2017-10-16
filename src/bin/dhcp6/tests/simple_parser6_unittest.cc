// Copyright (C) 2016-2017 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>
#include <gtest/gtest.h>
#include <dhcpsrv/parsers/simple_parser6.h>
#include <dhcp6/tests/dhcp6_test_utils.h>
#include <cc/data.h>

using namespace isc;
using namespace isc::data;
using namespace isc::dhcp;
using namespace isc::dhcp::test;

namespace {

/// @brief DHCP Parser test fixture class
class SimpleParser6Test : public ::testing::Test {
public:
    /// @brief Checks if specified map has an integer parameter with expected value
    ///
    /// @param map map to be checked
    /// @param param_name name of the parameter to be checked
    /// @param exp_value expected value of the parameter.
    void checkIntegerValue(const ConstElementPtr& map, const std::string& param_name,
                           int64_t exp_value) {

        // First check if the passed element is a map.
        ASSERT_EQ(Element::map, map->getType());

        // Now try to get the element being checked
        ConstElementPtr elem = map->get(param_name);
        ASSERT_TRUE(elem);

        // Now check if it's indeed integer
        ASSERT_EQ(Element::integer, elem->getType());

        // Finally, check if its value meets expectation.
        EXPECT_EQ(exp_value, elem->intValue());
    }

    /// @brief Checks if specified map has a string parameter with expected value
    ///
    /// @param map map to be checked
    /// @param param_name name of the parameter to be checked
    /// @param exp_value expected value of the parameter.
    void checkStringValue(const ConstElementPtr& map, const std::string& param_name,
                          std::string exp_value) {

        // First check if the passed element is a map.
        ASSERT_EQ(Element::map, map->getType());

        // Now try to get the element being checked
        ConstElementPtr elem = map->get(param_name);
        ASSERT_TRUE(elem);

        // Now check if it's indeed integer
        ASSERT_EQ(Element::string, elem->getType());

        // Finally, check if its value meets expectation.
        EXPECT_EQ(exp_value, elem->stringValue());
    }

    /// @brief Checks if specified map has a boolean parameter with expected value
    ///
    /// @param map map to be checked
    /// @param param_name name of the parameter to be checked
    /// @param exp_value expected value of the parameter.
    void checkBoolValue(const ConstElementPtr& map, const std::string& param_name,
                        bool exp_value) {

        // First check if the passed element is a map.
        ASSERT_EQ(Element::map, map->getType());

        // Now try to get the element being checked
        ConstElementPtr elem = map->get(param_name);
        ASSERT_TRUE(elem);

        // Now check if it's indeed integer
        ASSERT_EQ(Element::boolean, elem->getType());

        // Finally, check if its value meets expectation.
        EXPECT_EQ(exp_value, elem->boolValue());
    }

};

// This test checks if global defaults are properly set for DHCPv6.
TEST_F(SimpleParser6Test, globalDefaults6) {

    ElementPtr empty = parseJSON("{ }");
    size_t num = 0;

    EXPECT_NO_THROW(num = SimpleParser6::setAllDefaults(empty));

    // We expect at least 4 parameters to be inserted.
    EXPECT_TRUE(num >= 4);

    checkIntegerValue(empty, "valid-lifetime", 7200);
    checkIntegerValue(empty, "preferred-lifetime", 3600);
    checkIntegerValue(empty, "rebind-timer", 1800);
    checkIntegerValue(empty, "renew-timer", 900);
}

// This test checks if the parameters can be inherited from the global
// scope to the subnet scope.
TEST_F(SimpleParser6Test, inheritGlobalToSubnet6) {
    ElementPtr global = parseJSON("{ \"renew-timer\": 1,"
                                          "  \"rebind-timer\": 2,"
                                          "  \"preferred-lifetime\": 3,"
                                          "  \"valid-lifetime\": 4,"
                                          "  \"subnet6\": [ { \"renew-timer\": 100 } ] "
                                          "}");

    ConstElementPtr subnets = global->find("subnet6");
    ASSERT_TRUE(subnets);
    ConstElementPtr subnet = subnets->get(0);
    ASSERT_TRUE(subnet);

    // we should inherit 3 parameters. Renew-timer should remain intact,
    // as it was already defined in the subnet scope.
    size_t num;
    EXPECT_NO_THROW(num = SimpleParser6::deriveParameters(global));
    EXPECT_EQ(3, num);

    // Check the values. 3 of them are inherited, while the fourth one
    // was already defined in the subnet, so should not be inherited.
    checkIntegerValue(subnet, "renew-timer", 100);
    checkIntegerValue(subnet, "rebind-timer", 2);
    checkIntegerValue(subnet, "preferred-lifetime", 3);
    checkIntegerValue(subnet, "valid-lifetime", 4);
}

// This test checks if the parameters in "subnet6" are assigned default values
// if not explicitly specified.
TEST_F(SimpleParser6Test, subnetDefaults6) {
    ElementPtr global = parseJSON("{ \"renew-timer\": 1,"
                                  "  \"rebind-timer\": 2,"
                                  "  \"preferred-lifetime\": 3,"
                                  "  \"valid-lifetime\": 4,"
                                  "  \"subnet6\": [ { } ] "
                                  "}");

    size_t num = 0;
    EXPECT_NO_THROW(num = SimpleParser6::setAllDefaults(global));
    EXPECT_LE(1, num); // at least 1 parameter has to be modified

    ConstElementPtr subnets = global->find("subnet6");
    ASSERT_TRUE(subnets);
    ConstElementPtr subnet = subnets->get(0);
    ASSERT_TRUE(subnet);

    // we should have "id" parameter with the default value of 0 added for us.
    checkIntegerValue(subnet, "id", 0);
}

// This test checks if the parameters in option-data are assigned default values
// if not explicitly specified.
TEST_F(SimpleParser6Test, optionDataDefaults6) {
    ElementPtr global = parseJSON("{ \"renew-timer\": 1,"
                                  "  \"rebind-timer\": 2,"
                                  "  \"preferred-lifetime\": 3,"
                                  "  \"valid-lifetime\": 4,"
                                  "  \"option-data\": [ { } ] "
                                  "}");

    size_t num = 0;
    EXPECT_NO_THROW(num = SimpleParser6::setAllDefaults(global));
    EXPECT_LE(1, num); // at least 1 parameter has to be modified

    ConstElementPtr options = global->find("option-data");
    ASSERT_TRUE(options);
    ConstElementPtr option = options->get(0);
    ASSERT_TRUE(option);

    // we should have appropriate default value set. See
    // SimpleParser4::OPTION4_DEFAULTS for a list of default values.
    checkStringValue(option, "space", "dhcp6");
    checkBoolValue(option, "csv-format", true);
}

// This test checks if the parameters in option-data are assigned default values
// if not explicitly specified.
TEST_F(SimpleParser6Test, optionDefDefaults6) {
    ElementPtr global = parseJSON("{ "
                                  "    \"option-def\": [ { } ] "
                                  "}");

    size_t num = 0;
    EXPECT_NO_THROW(num = SimpleParser6::setAllDefaults(global));
    EXPECT_LE(1, num); // at least 1 parameter has to be modified

    ConstElementPtr defs = global->find("option-def");
    ASSERT_TRUE(defs);
    ASSERT_EQ(1, defs->size());
    ConstElementPtr def = defs->get(0);
    ASSERT_TRUE(def);

    // we should have appropriate default value set. See
    // SimpleParser4::OPTION4_DEFAULTS for a list of default values.
    checkStringValue(def, "record-types", "");
    checkStringValue(def, "space", "dhcp6");
    checkStringValue(def, "encapsulate", "");
    checkBoolValue(def, "array", false);
}


};

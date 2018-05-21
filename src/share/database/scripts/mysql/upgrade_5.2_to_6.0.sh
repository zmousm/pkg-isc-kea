#!/bin/sh

# Include utilities. Use installed version if available and
# use build version if it isn't.
if [ -e ${prefix}/share/kea/scripts/admin-utils.sh ]; then
    . ${prefix}/share/kea/scripts/admin-utils.sh
else
    . /home/wlodek/dev/kea_rel_1.4/src/bin/admin/admin-utils.sh
fi

VERSION=`mysql_version "$@"`

if [ "$VERSION" != "5.2" ]; then
    printf "This script upgrades 5.2 to 6.0. Reported version is $VERSION. Skipping upgrade.\n"
    exit 0
fi

mysql "$@" <<EOF

# Add user context into table holding hosts
ALTER TABLE hosts ADD COLUMN user_context TEXT NULL;

# Add user contexts into tables holding DHCP options
ALTER TABLE dhcp4_options ADD COLUMN user_context TEXT NULL;
ALTER TABLE dhcp6_options ADD COLUMN user_context TEXT NULL;

# Create index for searching leases by subnet identifier.
CREATE INDEX lease4_by_subnet_id ON lease4 (subnet_id);

# Create for searching leases by subnet identifier and lease type.
CREATE INDEX lease6_by_subnet_id_lease_type ON lease6 (subnet_id, lease_type);

# The index by iaid_subnet_id_duid is not the best choice because there are
# cases when we don't specify subnet identifier while searching leases. The
# index will be universal if the subnet_id is the right most column in the
# index.
DROP INDEX lease6_by_iaid_subnet_id_duid on lease6;
CREATE INDEX lease6_by_duid_iaid_subnet_id ON lease6 (duid, iaid, subnet_id);

# Create lease4_stat table
CREATE TABLE lease4_stat (
    subnet_id INT UNSIGNED NOT NULL,
    state INT UNSIGNED NOT NULL,
    leases BIGINT,
    PRIMARY KEY (subnet_id, state)
) ENGINE = INNODB;

# Create stat_lease4_insert trigger
DELIMITER $$
CREATE TRIGGER stat_lease4_insert AFTER INSERT ON lease4
    FOR EACH ROW
    BEGIN
        IF NEW.state = 0 OR NEW.state = 1 THEN
            # Update the state count if it exists
            UPDATE lease4_stat SET leases = leases + 1
            WHERE subnet_id = NEW.subnet_id AND state = NEW.state;

            # Insert the state count record if it does not exist
            IF ROW_COUNT() <= 0 THEN
                INSERT INTO lease4_stat VALUES (NEW.subnet_id, NEW.state, 1);
            END IF;
        END IF;
    END $$
DELIMITER ;

# Create stat_lease4_update trigger
DELIMITER $$
CREATE TRIGGER stat_lease4_update AFTER UPDATE ON lease4
    FOR EACH ROW
    BEGIN
        IF OLD.state != NEW.state THEN
            IF OLD.state = 0 OR OLD.state = 1 THEN
                # Decrement the old state count if record exists
                UPDATE lease4_stat SET leases = leases - 1
                WHERE subnet_id = OLD.subnet_id AND state = OLD.state;
            END IF;

            IF NEW.state = 0 OR NEW.state = 1 THEN
                # Increment the new state count if record exists
                UPDATE lease4_stat SET leases = leases + 1
                WHERE subnet_id = NEW.subnet_id AND state = NEW.state;

                # Insert new state record if it does not exist
                IF ROW_COUNT() <= 0 THEN
                    INSERT INTO lease4_stat VALUES (NEW.subnet_id, NEW.state, 1);
                END IF;
            END IF;
        END IF;
    END $$
DELIMITER ;

# Create stat_lease4_delete trigger
DELIMITER $$
CREATE TRIGGER stat_lease4_delete AFTER DELETE ON lease4
    FOR EACH ROW
    BEGIN
        IF OLD.state = 0 OR OLD.state = 1 THEN
            # Decrement the state count if record exists
            UPDATE lease4_stat SET leases = leases - 1
            WHERE subnet_id = OLD.subnet_id AND OLD.state = state;
        END IF;
    END $$
DELIMITER ;

# Create lease6_stat table
CREATE TABLE lease6_stat (
    subnet_id INT UNSIGNED NOT NULL,
    lease_type INT UNSIGNED NOT NULL,
    state INT UNSIGNED NOT NULL,
    leases BIGINT,
    PRIMARY KEY (subnet_id, lease_type, state)
) ENGINE = INNODB;

# Create stat_lease6_insert trigger
DELIMITER $$
CREATE TRIGGER stat_lease6_insert AFTER INSERT ON lease6
    FOR EACH ROW
    BEGIN
        IF NEW.state = 0 OR NEW.state = 1 THEN
            # Update the state count if it exists
            UPDATE lease6_stat SET leases = leases + 1
            WHERE
                subnet_id = NEW.subnet_id AND lease_type = NEW.lease_type
                AND state = NEW.state;

            # Insert the state count record if it does not exist
            IF ROW_COUNT() <= 0 THEN
                INSERT INTO lease6_stat
                VALUES (NEW.subnet_id, NEW.lease_type, NEW.state, 1);
            END IF;
        END IF;
    END $$
DELIMITER ;

# Create stat_lease6_update trigger
DELIMITER $$
CREATE TRIGGER stat_lease6_update AFTER UPDATE ON lease6
    FOR EACH ROW
    BEGIN
        IF OLD.state != NEW.state THEN
            IF OLD.state = 0 OR OLD.state = 1 THEN
                # Decrement the old state count if record exists
                UPDATE lease6_stat SET leases = leases - 1
                WHERE subnet_id = OLD.subnet_id AND lease_type = OLD.lease_type
                AND state = OLD.state;
            END IF;

            IF NEW.state = 0 OR NEW.state = 1 THEN
                # Increment the new state count if record exists
                UPDATE lease6_stat SET leases = leases + 1
                WHERE subnet_id = NEW.subnet_id AND lease_type = NEW.lease_type
                AND state = NEW.state;

                # Insert new state record if it does not exist
                IF ROW_COUNT() <= 0 THEN
                    INSERT INTO lease6_stat
                    VALUES (NEW.subnet_id, NEW.lease_type, NEW.state, 1);
                END IF;
            END IF;
        END IF;
    END $$
DELIMITER ;

# Create stat_lease6_delete trigger
DELIMITER $$
CREATE TRIGGER stat_lease6_delete AFTER DELETE ON lease6
    FOR EACH ROW
    BEGIN
        IF OLD.state = 0 OR OLD.state = 1 THEN
            # Decrement the state count if record exists
            UPDATE lease6_stat SET leases = leases - 1
            WHERE subnet_id = OLD.subnet_id AND lease_type = OLD.lease_type
            AND state = OLD.state;
        END IF;
    END $$
DELIMITER ;

# Populate lease4_stat table based on existing leases
# We only care about assigned and declined states
INSERT INTO lease4_stat (subnet_id, state, leases)
    SELECT subnet_id, state, count(state)
    FROM lease4 WHERE state < 2
    GROUP BY subnet_id, state ORDER BY subnet_id;

# Populate lease6_stat table based on existing leases
# We only care about assigned and declined states
INSERT INTO lease6_stat (subnet_id, lease_type, state, leases)
    SELECT subnet_id, lease_type, state, count(state)
    FROM lease6 WHERE state < 2
    GROUP BY subnet_id, lease_type, state
    ORDER BY subnet_id;

# Update the schema version number
UPDATE schema_version
SET version = '6', minor = '0';
# This line concludes database upgrade to version 6.0.

EOF

RESULT=$?

exit $?

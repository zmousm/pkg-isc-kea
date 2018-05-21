#!/bin/sh

# Include utilities. Use installed version if available and
# use build version if it isn't.
if [ -e ${prefix}/share/kea/scripts/admin-utils.sh ]; then
    . ${prefix}/share/kea/scripts/admin-utils.sh
else
    . /home/wlodek/dev/kea_rel_1.4/src/bin/admin/admin-utils.sh
fi

VERSION=`mysql_version "$@"`

if [ "$VERSION" != "5.1" ]; then
    printf "This script upgrades 5.1 to 5.2. Reported version is $VERSION. Skipping upgrade.\n"
    exit 0
fi

mysql "$@" <<EOF

# Make subnet_id column types consistent with lease table columns
ALTER TABLE dhcp4_options MODIFY dhcp4_subnet_id INT UNSIGNED;
ALTER TABLE dhcp6_options MODIFY dhcp6_subnet_id INT UNSIGNED;

# Update the schema version number
UPDATE schema_version
SET version = '5', minor = '2';
# This line concludes database upgrade to version 5.2.

EOF

RESULT=$?

exit $?

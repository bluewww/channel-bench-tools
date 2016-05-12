# extract_kd_off_log.awk
#
# Extract samples from test results, for the kernel deterministic scheduling, off time of the LOW domain
#
# This code is experimental, and error-handling is primitive.
#
# Copyright 2016, DATA61.  See COPYRIGHT for license details.

BEGIN           { running= 0 }
/offline time start/  { running= 1; next }
/offline time end/ { exit }
                { if(running) print }


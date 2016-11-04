# extract_kd_on_log.awk
#
# Extract samples from test results, for the kernel deterministic scheduling, on time of the LOW domain
#
# This code is experimental, and error-handling is primitive.
#
# Copyright 2016, DATA61.  See COPYRIGHT for license details.

BEGIN           { running= 0 }
/probing time start/  { running= 1; next }
/probing time end/ { exit }
                { if(running) print }


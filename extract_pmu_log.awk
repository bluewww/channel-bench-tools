# extract_kd_off_log.awk
#
# Extract samples from test results, for the kernel deterministic scheduling, off time of the LOW domain
#
# This code is experimental, and error-handling is primitive.
#
# Copyright 2016, DATA61.  See COPYRIGHT for license details.

BEGIN           { running= 0; pattern_s="pmu counter "number" start"; pattern_e="pmu counter "number" end"}
$0 ~ pattern_s { running= 1; next }
$0 ~ pattern_e { exit }
                { if(running) print }


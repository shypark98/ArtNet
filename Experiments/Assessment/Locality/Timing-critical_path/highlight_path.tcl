# This script was written and developed by ABKGroup students at UCSD.
# However, the underlying commands and reports are copyrighted by Cadence.
# We thank Cadence for granting permission to share our research
# to help promote and foster the next generation of innovators.

report_timing -max 1000 -output_format binary > foo.mtarpt

load_timing_debug_report foo.mtarpt

set path_list ""
for { set i 1 } { $i <= 20 } { incr i } {
    append path_list "$i "
}

setLayerPreference node_layer -isVisible 0

highlight_timing_report -path $path_list

dumpToGIF timing_path.png

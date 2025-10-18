read_lef ./Nangate45/Nangate45_tech.lef
read_lef ./Nangate45/Nangate45_stdcell.lef
read_liberty ./Nangate45/Nangate45_typ.lib

artnetgen_set_parameters \
    -seed 1 \
    -verbose 0 \
    -isAscend 1 \
    -minSeqBlocks 0 \
    -localConnect 0 \
    -sigmaTFactor 3.0 \
    -maxPathLen 40 \
    -defaultFlop DFF_X1


artnetgen_init \
    -spec_file large.spec \
    -netlist large.v \
    -print_netlist_info true

artnetgen_print_params

artnetgen_run

artnetgen_write_depthDist

artnetgen_clear

exit

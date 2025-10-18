set top_module ArtNet

read_lef ../pdk/lef/asap7_tech_1x_201209.lef
read_lef ../pdk/lef/asap7sc7p5t_27_R_1x_201211.lef
read_lef ../pdk/lef/fakeram7_128x32.lef

artnetgen_set_parameters \
    -seed 3 \
    -verbose 0 \
    -isAscend 0 \
    -minSeqBlocks 0 \
    -localConnect 1 \
    -sigmaTFactor 2 \
    -maxPathLen 70 \
    -minPathLen 1 \
    -macroMinPathLen 0 \
    -macroMaxPathLen 40 \
    -defaultFlop DFFHQNx2_ASAP7_75t_R

artnetgen_init \
    -spec_file ./${top_module}.spec \
    -netlist ${top_module}.v

artnetgen_run

artnetgen_clear

exit

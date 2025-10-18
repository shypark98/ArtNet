read_lef ../../../../pdk/lef/asap7_tech_1x_201209.lef
read_lef ../../../../pdk/lef/asap7sc7p5t_27_R_1x_201211.lef
read_lef ../../../../pdk/lef/asap7sc7p5t_27_L_1x_201211.lef
read_lef ../../../../pdk/lef/asap7sc7p5t_27_SL_1x_201211.lef

artnetgen_set_parameters \
    -seed 3 \
    -verbose 0 \
    -isAscend 0 \
    -minSeqBlocks 0 \
    -localConnect 1 \
    -sigmaTFactor 2 \
    -maxPathLen 88 \
    -minPathLen 1 \
    -localCutOff 70 \
    -flopCutOff 70 \
    -pathLenCutOff 70


artnetgen_init \
    -spec_file ./nova.spec \
    -netlist nova.v

artnetgen_print_params

artnetgen_run

artnetgen_clear

exit

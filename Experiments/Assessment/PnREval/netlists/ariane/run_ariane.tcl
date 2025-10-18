read_lef ../../../../pdk/lef/asap7_tech_1x_201209.lef
read_lef ../../../../pdk/lef/asap7sc7p5t_27_R_1x_201211.lef
read_lef ../../../../pdk/lef/asap7sc7p5t_27_L_1x_201211.lef
read_lef ../../../../pdk/lef/asap7sc7p5t_27_SL_1x_201211.lef
read_lef ../../../../pdk/lef/sram_asap7_16x256_1rw.lef

artnetgen_set_parameters \
    -seed 3 \
    -verbose 0 \
    -isAscend 0 \
    -minSeqBlocks 0 \
    -localConnect 1 \
    -sigmaTFactor 2 \
    -maxPathLen 291 \
    -minPathLen 0 \
    -macroMinPathLen 0 \
    -macroMaxPathLen 85 \
    -localCutOff 70 \
    -flopCutOff 70 \
    -pathLenCutOff 70 \
    -defaultFlop DFFHQNx1_ASAP7_75t_R

artnetgen_init \
    -spec_file ariane.spec \

artnetgen_set_macro -master sram_asap7_16x256_1rw

artnetgen_run

artnetgen_clear

exit

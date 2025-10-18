set top_module ArtNet
set num_insts 2000000
set num_macros 5
set region_one 0.45
set num_pi 20
set num_po 20
set avg_fanin 2.5
set p 0.55
set q 0.10
set seq_ratio 0.20
set seed 3
set result_path .

read_lef ../../../../../Exps/pdk/lef/asap7_tech_1x_201209.lef
read_lef ../../../../../Exps/pdk/lef/asap7sc7p5t_27_R_1x_201211.lef

artnetgen_create_spec -top_module ${top_module} \
                      -num_inst ${num_insts} \
                      -num_macros ${num_macros} \
                      -region_one ${region_one} \
                      -num_pi ${num_pi} \
                      -num_po ${num_po} \
                      -avg_fi ${avg_fanin} \
                      -p ${p} \
                      -q ${q} \
                      -seq_ratio ${seq_ratio} \
                      -seed ${seed} \
                      -out_file ${result_path}/${top_module}.spec

artnetgen_set_parameters \
    -seed 3 \
    -verbose 0 \
    -isAscend 0 \
    -minSeqBlocks 0 \
    -localConnect 1 \
    -sigmaTFactor 2 \
    -maxPathLen 40 \
    -minPathLen 1 \
    -localCutOff 70 \
    -flopCutOff 70 \
    -defaultFlop DFFHQNx1_ASAP7_75t_R

artnetgen_init \
    -spec_file ./${top_module}.spec \
    -netlist ${top_module}.v

artnetgen_run

artnetgen_clear

exit

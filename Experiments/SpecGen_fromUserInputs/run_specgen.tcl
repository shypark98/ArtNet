set top_module ArtNet
set num_insts 2000000
set num_macros 5
set region_one 0.45
set num_pi 5000
set num_po 5000
set avg_fanin 2.5
set p 0.55
set q 0.10
set seq_ratio 0.20
set seed 3
set result_path .

read_lef ../pdk/lef/asap7_tech_1x_201209.lef
read_lef ../pdk/lef/asap7sc7p5t_27_R_1x_201211.lef
read_lef ../pdk/lef/fakeram7_128x32.lef

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

exit

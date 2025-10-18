set top_module __TOP_MODULE__
set def_file __DEF_FILE__
set netlist_file __NETLIST_FILE__
set site asap7sc7p5t
set core_utilization 10
set aspect_ratio 1.0
set core_margin 0.1
set io_placer_hor_layer M6
set io_placer_ver_layer M5
set min_routing_layer M2
set max_routing_layer M7
set pdk_dir ../pdk
set lib_dir ./utils

read_lef ${pdk_dir}/lef/asap7_tech_1x_201209.lef
read_lef ${pdk_dir}/lef/asap7sc7p5t_27_L_1x_201211.lef
read_lef ${pdk_dir}/lef/asap7sc7p5t_27_R_1x_201211.lef
read_lef ${pdk_dir}/lef/asap7sc7p5t_27_SL_1x_201211.lef
read_lef ${pdk_dir}/lef/sram_asap7_16x256_1rw.lef


read_liberty ${pdk_dir}/lib/asap7sc7p5t_AO_LVT_TT_nldm_201020.lib
read_liberty ${pdk_dir}/lib/asap7sc7p5t_AO_RVT_TT_nldm_201020.lib
read_liberty ${pdk_dir}/lib/asap7sc7p5t_AO_SLVT_TT_nldm_201020.lib

read_liberty ${pdk_dir}/lib/asap7sc7p5t_INVBUF_LVT_TT_nldm_201020.lib
read_liberty ${pdk_dir}/lib/asap7sc7p5t_INVBUF_RVT_TT_nldm_201020.lib
read_liberty ${pdk_dir}/lib/asap7sc7p5t_INVBUF_SLVT_TT_nldm_201020.lib

read_liberty ${pdk_dir}/lib/asap7sc7p5t_OA_LVT_TT_nldm_201020.lib
read_liberty ${pdk_dir}/lib/asap7sc7p5t_OA_RVT_TT_nldm_201020.lib
read_liberty ${pdk_dir}/lib/asap7sc7p5t_OA_SLVT_TT_nldm_201020.lib

read_liberty ${pdk_dir}/lib/asap7sc7p5t_SEQ_LVT_TT_nldm_201020.lib
read_liberty ${pdk_dir}/lib/asap7sc7p5t_SEQ_RVT_TT_nldm_201020.lib
read_liberty ${pdk_dir}/lib/asap7sc7p5t_SEQ_SLVT_TT_nldm_201020.lib

read_liberty ${pdk_dir}/lib/asap7sc7p5t_SIMPLE_LVT_TT_nldm_201020.lib
read_liberty ${pdk_dir}/lib/asap7sc7p5t_SIMPLE_RVT_TT_nldm_201020.lib
read_liberty ${pdk_dir}/lib/asap7sc7p5t_SIMPLE_SLVT_TT_nldm_201020.lib

read_liberty ${pdk_dir}/lib/sram_asap7_16x256_1rw.lib


read_verilog ${netlist_file}

link_design $top_module
read_sdc ${lib_dir}/ref.sdc 

initialize_floorplan -site $site \
    -utilization $core_utilization \
    -aspect_ratio $aspect_ratio \
    -core_space $core_margin \

#place_pins -random -hor_layers $io_placer_hor_layer -ver_layers $io_placer_ver_layer

#set_global_routing_layer_adjustment $min_routing_layer-$max_routing_layer 0.5
#set_routing_layers -signal $min_routing_layer-$max_routing_layer

set place_density_lb [gpl::get_global_placement_uniform_density]
set target_density [expr $place_density_lb + (1 - $place_density_lb) * 0.5]
global_placement -skip_io -density $target_density

write_def ${def_file}

exit

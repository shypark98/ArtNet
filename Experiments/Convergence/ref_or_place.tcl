############################################################################
##
## BSD 3-Clause License
##
## Copyright (c) 2025, Seonghyeon Park and the Regents of the University of California
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##
## * Redistributions of source code must retain the above copyright notice, this
##   list of conditions and the following disclaimer.
##
## * Redistributions in binary form must reproduce the above copyright notice,
##   this list of conditions and the following disclaimer in the documentation
##   and/or other materials provided with the distribution.
##
## * Neither the name of the copyright holder nor the names of its
##   contributors may be used to endorse or promote products derived from
##   this software without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
## LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
## CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
## SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
## CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.
##
############################################################################

set top_module __TOP_MODULE__
set def_dir __DEF_DIR__
set netlist_dir __NETLIST_DIR__
set site asap7sc7p5t
set core_utilization 50
set aspect_ratio 1.0
set core_margin 0.1
set io_placer_hor_layer M6
set io_placer_ver_layer M5
set min_routing_layer M2
set max_routing_layer M7


read_lef ../pdk/lef/asap7_tech_1x_201209.lef
read_lef ../pdk/lef/asap7sc7p5t_27_L_1x_201211.lef
read_lef ../pdk/lef/asap7sc7p5t_27_R_1x_201211.lef
read_lef ../pdk/lef/asap7sc7p5t_27_SL_1x_201211.lef

read_liberty ../pdk/lib/asap7sc7p5t_AO_LVT_TT_nldm_201020.lib
read_liberty ../pdk/lib/asap7sc7p5t_AO_RVT_TT_nldm_201020.lib
read_liberty ../pdk/lib/asap7sc7p5t_AO_SLVT_TT_nldm_201020.lib

read_liberty ../pdk/lib/asap7sc7p5t_INVBUF_LVT_TT_nldm_201020.lib
read_liberty ../pdk/lib/asap7sc7p5t_INVBUF_RVT_TT_nldm_201020.lib
read_liberty ../pdk/lib/asap7sc7p5t_INVBUF_SLVT_TT_nldm_201020.lib

read_liberty ../pdk/lib/asap7sc7p5t_OA_LVT_TT_nldm_201020.lib
read_liberty ../pdk/lib/asap7sc7p5t_OA_RVT_TT_nldm_201020.lib
read_liberty ../pdk/lib/asap7sc7p5t_OA_SLVT_TT_nldm_201020.lib

read_liberty ../pdk/lib/asap7sc7p5t_SEQ_LVT_TT_nldm_201020.lib
read_liberty ../pdk/lib/asap7sc7p5t_SEQ_RVT_TT_nldm_201020.lib
read_liberty ../pdk/lib/asap7sc7p5t_SEQ_SLVT_TT_nldm_201020.lib

read_liberty ../pdk/lib/asap7sc7p5t_SIMPLE_LVT_TT_nldm_201020.lib
read_liberty ../pdk/lib/asap7sc7p5t_SIMPLE_RVT_TT_nldm_201020.lib
read_liberty ../pdk/lib/asap7sc7p5t_SIMPLE_SLVT_TT_nldm_201020.lib

read_verilog ${netlist_dir}/${top_module}.v

link_design $top_module
read_sdc ./ref.sdc 

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

write_def ${def_dir}/${top_module}_preRoute.def

exit

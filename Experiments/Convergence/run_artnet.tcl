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
set num_insts __NUM_INSTS__ 
set region_one __REGION_ONE__
set num_pi __NUM_PI__
set num_po __NUM_PO__
set avg_fanin __AVG_FANIN__
set p __P__
set q __Q__
set seq_ratio __SEQ_RATIO__
set seed 3
set result_path __RESULT_PATH__

set lef_path ../pdk/lef

read_lef ${lef_path}/asap7_tech_1x_201209.lef
read_lef ${lef_path}/asap7sc7p5t_27_L_1x_201211.lef
read_lef ${lef_path}/asap7sc7p5t_27_R_1x_201211.lef
read_lef ${lef_path}/asap7sc7p5t_27_SL_1x_201211.lef

artnetgen_create_spec -top_module ${top_module} \
                      -num_inst ${num_insts} \
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

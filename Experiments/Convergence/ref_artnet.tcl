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
set result_path __RESULT_PATH__

set lef_path ../pdk/lef

read_lef ${lef_path}/asap7_tech_1x_201209.lef
read_lef ${lef_path}/asap7sc7p5t_27_L_1x_201211.lef
read_lef ${lef_path}/asap7sc7p5t_27_R_1x_201211.lef
read_lef ${lef_path}/asap7sc7p5t_27_SL_1x_201211.lef

artnetgen_set_parameters \
    -seed 1 \
    -verbose 0 \
    -minSeqBlocks 0 \
    -localConnect 0 \
    -sigmaTFactor 5 \
    -maxPathLen 40 \
    -minPathLen 1 \
    -pathLenCutOff 70 \
    -allowLoop 0

artnetgen_init \
    -spec_file ${result_path}/${top_module}.spec \

artnetgen_write_verilog

artnetgen_run

artnetgen_clear


exit

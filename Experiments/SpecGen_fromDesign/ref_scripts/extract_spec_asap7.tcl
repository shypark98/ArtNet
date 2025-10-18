source "./utils/helpers.tcl"

set work_dir __WORK_DIR__
set design __DESIGN__
set spec_dir __SPEC_DIR__
set verilog_dir __VERI_DIR__
set lib_dir ../pdk

if { ![file exists $spec_dir] } {
    file mkdir $spec_dir
}

if { ![file exists $verilog_dir] } {
    file mkdir $verilog_dir
}

read_liberty ${lib_dir}/lib/asap7sc7p5t_AO_LVT_TT_nldm_201020.lib
read_liberty ${lib_dir}/lib/asap7sc7p5t_AO_RVT_TT_nldm_201020.lib
read_liberty ${lib_dir}/lib/asap7sc7p5t_AO_SLVT_TT_nldm_201020.lib
read_liberty ${lib_dir}/lib/asap7sc7p5t_INVBUF_LVT_TT_nldm_201020.lib
read_liberty ${lib_dir}/lib/asap7sc7p5t_INVBUF_RVT_TT_nldm_201020.lib
read_liberty ${lib_dir}/lib/asap7sc7p5t_INVBUF_SLVT_TT_nldm_201020.lib
read_liberty ${lib_dir}/lib/asap7sc7p5t_OA_LVT_TT_nldm_201020.lib
read_liberty ${lib_dir}/lib/asap7sc7p5t_OA_RVT_TT_nldm_201020.lib
read_liberty ${lib_dir}/lib/asap7sc7p5t_OA_SLVT_TT_nldm_201020.lib
read_liberty ${lib_dir}/lib/asap7sc7p5t_SEQ_LVT_TT_nldm_201020.lib
read_liberty ${lib_dir}/lib/asap7sc7p5t_SEQ_RVT_TT_nldm_201020.lib
read_liberty ${lib_dir}/lib/asap7sc7p5t_SEQ_SLVT_TT_nldm_201020.lib
read_liberty ${lib_dir}/lib/asap7sc7p5t_SIMPLE_LVT_TT_nldm_201020.lib
read_liberty ${lib_dir}/lib/asap7sc7p5t_SIMPLE_RVT_TT_nldm_201020.lib
read_liberty ${lib_dir}/lib/asap7sc7p5t_SIMPLE_SLVT_TT_nldm_201020.lib
read_liberty ${lib_dir}/lib/sram_asap7_16x256_1rw.lib

read_lef ${lib_dir}/lef/asap7_tech_1x_201209.lef
read_lef ${lib_dir}/lef/asap7sc7p5t_27_R_1x_201211.lef
read_lef ${lib_dir}/lef/asap7sc7p5t_27_L_1x_201211.lef
read_lef ${lib_dir}/lef/asap7sc7p5t_27_SL_1x_201211.lef
read_lef ${lib_dir}/lef/sram_asap7_16x256_1rw.lef

read_verilog ${work_dir}/${design}.v
link_design ${design}

read_sdc ./utils/ref.sdc

artnetgen_write_spec -out_file ${spec_dir}/${design}.spec \
                     -out_verilog_dir ${verilog_dir} \
                     -is_flat false \
                     -hierarchy_level 2

exit

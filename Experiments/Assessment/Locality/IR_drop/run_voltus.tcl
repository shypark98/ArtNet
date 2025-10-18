# This script was written and developed by ABKGroup students at UCSD. 
# However, the underlying commands and reports are copyrighted by Cadence. 
# We thank Cadence for granting permission to share our research to help 
# promote and foster the next generation of innovators.

setMultiCpuUsage -localCpu 4

set design nova

set lef [list asap7_tech_1x_201209.lef asap7sc7p5t_27_L_1x_201211.lef asap7sc7p5t_27_R_1x_201211.lef asap7sc7p5t_27_SL_1x_201211.lef]

set def ${design}.def
set qrc "ASAP7.tch"

read_lib -lef $lef
read_view_definition viewDefinition.tcl
read_verilog ${design}.v
set_top_module ${design} -ignore_undefined_cell

read_def $def

set_pg_nets -net VDD -voltage 0.7 -threshold 0.4 -force
set_pg_nets -net VSS -voltage 0.0 -threshold 0.3 -force

read_spef ${design}.spef
specify_spef ${design}.spef

set_power_analysis_mode -reset
set_power_pads -reset
set_power_data -reset

set_pg_library_mode -celltype techonly \
  -default_area_cap 0.5 \
  -extraction_tech_file $qrc \
  -power_pins {VDD 0.7} \
  -ground_pins VSS

generate_pg_library 

set_power_analysis_mode -method static \
  -corner max \
  -create_binary_db true \
  -write_static_currents true

set_power_output_dir ./static_power        

report_power

set_rail_analysis_mode -method static \
  -power_switch_eco false \
  -accuracy hd -analysis_view AV_wc_on \
  -power_grid_library "techonly.cl" \
  -process_techgen_em_rules false \
  -enable_xp false -enable_rlrp_analysis true \
  -extraction_tech_file $qrc
#  -report_power_in_parallel true

set_pg_nets -net VDD -voltage 0.7 -threshold 0.4 -force
set_pg_nets -net VSS -voltage 0.0 -threshold 0.3 -force
set_rail_analysis_domain -name core -pwrnets VDD -gndnets VSS
set_power_pads -net VDD -format xy -file VDD.vsrc
set_power_pads -net VSS -format xy -file VSS.vsrc

set_power_data -format current -scale 1 { static_power/static_VDD.ptiavg static_power/static_VSS.ptiavg}

analyze_rail -output static_IR -type domain core

save_design latest


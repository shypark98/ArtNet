# This script was written and developed by ABKGroup students at UCSD. 
# However, the underlying commands and reports are copyrighted by Cadence. 
# We thank Cadence for granting permission to share our research to help 
# promote and foster the next generation of innovators.

proc create_dir { dir_path } {
    if { ![file exists $dir_path] } {
        file mkdir ${dir_path}
    }
}

set libdir "../pdk/lib"
set lefdir "../pdk/lef"
set qrcdir "../pdk/qrc"

set_db init_lib_search_path { \
    ${libdir} \
    ${lefdir} \
}

set libworst [glob ${libdir}/*.lib]

set libbest $libworst

set lefs "
    ${lefdir}/asap7_tech_1x_201209.lef \
    ${lefdir}/asap7sc7p5t_27_R_1x_201211.lef \
    ${lefdir}/asap7sc7p5t_27_L_1x_201211.lef \
    ${lefdir}/asap7sc7p5t_27_SL_1x_201211.lef \
    ${lefdir}/asap7sc7p5t_27_SRAM_1x_201211.lef \
    "

set qrc_max "${qrcdir}/ASAP7.tch"
set qrc_min "${qrcdir}/ASAP7.tch"

setMultiCpuUsage -localCpu 16
setDesignMode -process 7

set top_module      __TOP_MODULE__
set run_dir         __WORK_HOME__
set aspect_ratio    __ASPECT_RATIO__
set layout_util     __UTILIZATION__
set max_route_layer __RMAX_LYR__
set pnr_clk_period  __CLK_PERIOD__
set pdn_type        __PDN_TYPE__
set floorplan "util_${layout_util}_ar_${aspect_ratio}_rmax_${max_route_layer}_pdn_${pdn_type}_clk_${pnr_clk_period}"
set bench_dir       __BENCH_DIR__
set db_dir          ${bench_dir}/enc/${floorplan}
set rpt_dir         ${bench_dir}/rpt/${floorplan}
set spef_dir        ${bench_dir}/spef/${floorplan}
set drc_dir         ${bench_dir}/drc/${floorplan}
set def_dir         ${bench_dir}/def/${floorplan}
set net_dir         ${bench_dir}/netlist
set work_dir        ${run_dir}/work_dir/${top_module}_${floorplan}

if {![file exists ${bench_dir}/enc/]} {
    exec mkdir ${bench_dir}/enc/
}

if {![file exists ${bench_dir}/rpt/]} {
    exec mkdir ${bench_dir}/rpt/
}

if {![file exists ${bench_dir}/spef/]} {
    exec mkdir ${bench_dir}/spef/
}

if {![file exists ${bench_dir}/drc/]} {
    exec mkdir ${bench_dir}/drc/
}

if {![file exists ${bench_dir}/def/]} {
    exec mkdir ${bench_dir}/def/
}

if {![file exists ${bench_dir}/work_dir/]} {
    exec mkdir ${bench_dir}/work_dir/
}

create_dir $db_dir
create_dir $rpt_dir
create_dir $spef_dir
create_dir $work_dir
create_dir $drc_dir
create_dir $def_dir

cd $work_dir

#1 Set Libraries
set netlist ${net_dir}/${top_module}.v
set sdc     ${net_dir}/${top_module}_${pnr_clk_period}.sdc

setLibraryUnit -time 1ps

create_library_set -name WC_LIB -timing $libworst
create_library_set -name BC_LIB -timing $libbest

create_rc_corner -name Cmax -qx_tech_file $qrc_max
create_rc_corner -name Cmin -qx_tech_file $qrc_min


create_delay_corner -name WC -library_set WC_LIB -rc_corner Cmax
create_delay_corner -name BC -library_set BC_LIB -rc_corner Cmin

create_constraint_mode -name CON -sdc_file $sdc
create_analysis_view -name WC_VIEW -delay_corner WC -constraint_mode CON
create_analysis_view -name BC_VIEW -delay_corner BC -constraint_mode CON


# default settings
set init_pwr_net VDD
set init_gnd_net VSS

# default settings
set init_verilog "$netlist"
set init_design_netlisttype "Verilog"
set init_design_settop 1
set init_top_cell "$top_module"
set init_lef_file "$lefs"

# MCMM setup
init_design -setup {WC_VIEW} -hold {BC_VIEW}
set_power_analysis_mode -leakage_power_view WC_VIEW -dynamic_power_view WC_VIEW

set_interactive_constraint_modes {CON}
setAnalysisMode -reset
setAnalysisMode -analysisType onChipVariation -cppr both

clearGlobalNets
globalNetConnect VDD -type pgpin -pin VDD -inst * -override
globalNetConnect VSS -type pgpin -pin VSS -inst * -override
globalNetConnect VDD -type tiehi -inst * -override
globalNetConnect VSS -type tielo -inst * -override


setOptMode -powerEffort low -leakageToDynamicRatio 0.5
setGenerateViaMode -auto true
generateVias

# basic path groups
createBasicPathGroups -expanded

## Generate the floorplan ##
setFPlanMode -snapBlockGrid LayerTrack

floorPlan -r $aspect_ratio $layout_util 0.2 0.2 0.2 0.2
### Write postSynth report ###
echo "Physical Design Stage, Core Area (um^2), Standard Cell Area (um^2), Macro Area (um^2), Total Power (mW), Wirelength(um), WS(ns), TNS(ns), Congestion(H), Congestion(V)" > ${top_module}_DETAILS.rpt
source ./util/extract_report.tcl
set rpt_post_synth [extract_report postSynth]
echo "$rpt_post_synth" >> ${rpt_dir}/${top_module}_DETAILS.rpt

### Add power plan ###
source ./pdn/pdn_config.tcl
source ./pdn/pdn_flow.tcl
deleteBufferTree
#saveDesign -compress ${db_dir}/${top_module}_floorplan.enc

setPlaceMode -place_detail_legalization_inst_gap 1
setPlaceMode -place_global_place_io_pins true
setFillerMode -fitGap true
setDesignMode -topRoutingLayer $max_route_layer
setDesignMode -bottomRoutingLayer 2 

#place_opt_design -out_dir $rpt_dir -prefix place
place_opt_design

## Write out the required info for blob placement ##
setExtractRCMode -engine preRoute
extractRC
rcOut -spef ${spef_dir}/${top_module}_preRoute.spef

set lefDefOutVersion 5.8
defOut -earlyGlobalRoute ${def_dir}/${top_module}_preRoute.def

write_global_slack_report -late > ${rpt_dir}/global_slack_report.txt
saveNetlist ${net_dir}/${top_module}_buf_removed.v
#saveDesign -compress $db_dir/${top_module}_place.enc

set rpt_pre_cts [extract_report preCTS]
echo "$rpt_pre_cts" >> ${rpt_dir}/${top_module}_DETAILS.rpt

set_ccopt_property post_conditioning_enable_routing_eco 1
set_ccopt_property -cts_def_lock_clock_sinks_after_routing true
setOptMode -unfixClkInstForOpt false

create_ccopt_clock_tree_spec
ccopt_design

set_interactive_constraint_modes [all_constraint_modes -active]
set_propagated_clock [all_clocks]
set_clock_propagation propagated

#saveDesign -compress $db_dir/${top_module}_cts.enc
set rpt_post_cts [extract_report postCTS]
echo "$rpt_post_cts" >> ${rpt_dir}/${top_module}_DETAILS.rpt


# ------------------------------------------------------------------------------
# Routing
# ------------------------------------------------------------------------------
setNanoRouteMode -drouteVerboseViolationSummary 1
setNanoRouteMode -routeWithSiDriven true
setNanoRouteMode -routeWithTimingDriven true
setNanoRouteMode -routeUseAutoVia true

##Recommended by lib owners
# Prevent router modifying M1 pins shapes
setNanoRouteMode -routeWithViaInPin "1:1"
setNanoRouteMode -routeWithViaOnlyForStandardCellPin "1:1"

## limit VIAs to ongrid only for VIA1 (S1)
setNanoRouteMode -drouteOnGridOnly "via 1:1"
setNanoRouteMode -drouteAutoStop false
setNanoRouteMode -drouteExpAdvancedMarFix true
setNanoRouteMode -routeExpAdvancedTechnology true

#SM suggestion for solving long extraction runtime during GR
setNanoRouteMode -grouteExpWithTimingDriven false

routeDesign
#route_opt_design
#saveDesign -compress ${db_dir}/${top_module}_route.enc

### Add V1 vias ###
setViaGenMode -reset
editPowerVia -top_layer M2 -bottom_layer M1 -orthogonal_only 0 -add_vias 1

### Run DRC and LVS ###
set_verify_drc_mode -report ${drc_dir}/postRoute.txt
verify_connectivity -error 0 -geom_connect -no_antenna
verify_drc -limit 0

set lefDefOutVersion 5.8
set rpt_post_route [extract_report postRoute]
echo "$rpt_post_route" >> ${rpt_dir}/${top_module}_DETAILS.rpt
defOut -netlist -floorplan -routing ${def_dir}/${top_module}_Route.def

#route_opt_design
optDesign -postRoute
set rpt_post_route [extract_report postRouteOpt]
echo "$rpt_post_route" >> ${rpt_dir}/${top_module}_DETAILS.rpt

summaryReport -noHtml -outfile summaryReport/post_route.sum
saveDesign -compress ${db_dir}/${top_module}_routeOpt.enc
set lefDefOutVersion 5.8
defOut -netlist -floorplan -routing ${def_dir}/${top_module}_postRoute.def

set_verify_drc_mode -report ${drc_dir}/postRouteOpt.txt
verify_connectivity -error 0 -geom_connect -no_antenna
verify_drc -limit 0
set exec_date [clock format [clock seconds] -format %m_%d_%H_%M_%S]

exit

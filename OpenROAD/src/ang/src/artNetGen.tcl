###############################################################################
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
##   and#or other materials provided with the distribution.
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
################################################################################

sta::define_cmd_args "artnetgen_create_spec" {
    [-top_module] \
    [-num_inst] \
    [-num_macros] \
    [-num_pi] \
    [-num_po] \
    [-region_one] \
    [-avg_fi] \
    [-p] \
    [-q] \
    [-meanG] \
    [-sigmaG] \
    [-seq_ratio] \
    [-seed] \
    [-cell_list] \
    [-out_file]
}

proc artnetgen_create_spec { args } {
    sta::parse_key_args "artnetgen_create_spec" args \
        keys { -top_module \
               -num_inst \
               -num_macros \
               -num_pi \
               -num_po \
               -region_one \
               -avg_fi \
               -p \
               -q \
               -meanG \
               -sigmaG \
               -seq_ratio \
               -seed \
               -cell_list \
               -out_file } flags {}
    
    set top_module "top_module"
    set num_inst 100000
    set num_macros 0
    set num_pi 50
    set num_po 50
    set avg_fi 2.5
    set p 0.65
    set q 0.1
    set meanG 0.3
    set sigmaG 0.1
    set seq_ratio 0.1
    set region_one 0.45
    set seed 1
    set cell_list ""


    if { [info exist keys(-top_module)] } {
        set top_module $keys(-top_module)
    }

    if { [info exist keys(-num_inst)] } {
        set num_inst $keys(-num_inst)
    }

    if { [info exist keys(-num_macros)] } {
        set num_macros $keys(-num_macros)
    }

    if { [info exist keys(-num_pi)] } {
        set num_pi $keys(-num_pi)
    }

    if { [info exist keys(-num_po)] } {
        set num_po $keys(-num_po)
    }
    
    if { [info exist keys(-region_one)] } {
        set region_one $keys(-region_one)
    }

    if { [info exist keys(-avg_fi)] } {
        set avg_fi $keys(-avg_fi)
    }

    if { [info exist keys(-p)] } {
        set p $keys(-p)
    }

    if { [info exist keys(-q)] } {
        set q $keys(-q)
    }

    if { [info exist keys(-meanG)] } {
        set meanG $keys(-meanG)
    }

    if { [info exist keys(-sigmaG)] } {
        set sigmaG $keys(-sigmaG)
    }

    if { [info exist keys(-seq_ratio)] } {
        set seq_ratio $keys(-seq_ratio)
    }
    
    if { [info exist keys(-seed)] } {
        set seed $keys(-seed)
    }
    
    if { [info exist keys(-cell_list)] } {
        set cell_list $keys(-cell_list)
    }
    
    set out_file $keys(-out_file)

 
    sta::check_positive_integer "-num_inst" $num_inst
    sta::check_positive_integer "-num_macros" $num_macros
    sta::check_positive_integer "-num_pi" $num_pi
    sta::check_positive_integer "-num_po" $num_po
    sta::check_positive_integer "-seed" $seed
    sta::check_positive_float "-region_one" $region_one
    sta::check_positive_float "-avg_fi" $avg_fi
    sta::check_positive_float "-p" $p
    sta::check_positive_float "-q" $q
    sta::check_positive_float "-meanG" $meanG
    sta::check_positive_float "-sigmaG" $sigmaG
    sta::check_positive_float "-seq_ratio" $seq_ratio

    artnetgen_create_spec_cmd $top_module \
                              $num_inst \
                              $num_macros \
                              $num_pi \
                              $num_po \
                              $region_one \
                              $avg_fi \
                              $meanG \
                              $sigmaG \
                              $p \
                              $q \
                              $seq_ratio \
                              $seed \
                              $cell_list \
                              $out_file

}



sta::define_cmd_args "artnetgen_write_spec" {
    [-out_file] \
    [-out_verilog_dir] \
    [-is_flat] \
    [-hierarchy_level] \
    [-mini_brain] \
    [-min_insts]
}

proc artnetgen_write_spec { args } {
    sta::parse_key_args "artnetgen_write_spec" args \
        keys { -out_file \
               -out_verilog_dir \
               -is_flat \
               -hierarchy_level \
               -mini_brain \
               -min_insts } flags {}
    
    set out_file "ArtNet.spec"
    set hierarchy_level 0
    set out_verilog_dir "."
    set is_flat false
    set mini_brain 1.0
    set min_insts 100
    
    if { [info exists keys(-out_file)] } {
        set out_file $keys(-out_file)
    }

    if { [info exists keys(-out_verilog_dir)] } {
        set out_verilog_dir $keys(-out_verilog_dir)
    }
    
    if { [info exists keys(-is_flat)] } {
        set is_flat $keys(-is_flat)
    }

    if { [info exists keys(-hierarchy_level)] } {
        set hierarchy_level $keys(-hierarchy_level)
    }
    
    if { [info exists keys(-mini_brain)] } {
        set mini_brain $keys(-mini_brain)
    }
    
    if { [info exists keys(-min_insts)] } {
        set min_insts $keys(-min_insts)
    }

    sta::check_positive_float "-mini_brain" $mini_brain
    sta::check_positive_integer "-hierarchy_level" $hierarchy_level
    sta::check_positive_integer "-min_insts" $min_insts

    artnetgen_write_spec_cmd $out_file $out_verilog_dir $is_flat $hierarchy_level $mini_brain $min_insts
}

proc artnetgen_write_depthDist { } {
    set flag true

    artnetgen_set_printDepth_cmd $flag
}

proc artnetgen_report_metrics { } {
    artnetgen_report_metrics_cmd
}


sta::define_cmd_args "artnetgen_set_dont_use" {
    [-master]
}

proc artnetgen_set_dont_use { args } {
    sta::parse_key_args "artnetgen_set_dont_use" args \
        keys { -master } flags {}
    
    if { [info exists keys(-master)] } {
        set master $keys(-master)
        artnetgen_set_dont_use_cmd $master
    }
}


sta::define_cmd_args "artnetgen_set_macro" {
    [-master]
}

proc artnetgen_set_macro { args } {
    sta::parse_key_args "artnetgen_set_macro" args \
        keys { -master } flags {}
    
    if { [info exists keys(-master)] } {
        set master $keys(-master)
        artnetgen_set_macro_cmd $master
    }

}



sta::define_cmd_args "artnetgen_set_parameters" {
    [-seed] \
    [-verbose] \
    [-allowLoops] \
    [-isAscend] \
    [-localConnect] \
    [-insertFlop] \
    [-sigmaTFactor] \
    [-minSeqBlocks] \
    [-localCutoff] \
    [-minInputs] \
    [-minOutputs] \
    [-minPathLen] \
    [-maxPathLen] \
    [-macroMinPathLen] \
    [-macroMaxPathLen] \
    [-pathLenCutOff] \
    [-flopCutOff] \
    [-flopDam] \
    [-distThreshold] \
    [-distMeanT] \
    [-distMeanG] \
    [-distFactor] \
    [-flopInsertProb] \
    [-defaultFlop]

}

proc artnetgen_set_parameters { args } {
    sta::parse_key_args "artnetgen_set_parameters" args \
        keys { -seed \
               -verbose \
               -allowLoops \
               -isAscend \ 
               -localConnect \
               -insertFlop \
               -sigmaTFactor \
               -minSeqBlocks \
               -localCutOff \
               -minInputs \
               -minOutputs \
               -minPathLen \
               -maxPathLen \
               -macroMinPathLen \
               -macroMaxPathLen \
               -pathLenCutOff \
               -flopCutOff \
               -flopDam \
               -distThreshold \
               -distMeanT \
               -distMeanG \
               -distFactor \
               -flopInsertProb \
               -defaultFlop \
            } flags {}
    
    set seed 1
    set verbose false
    set allowLoops false
    set isAscend false
    set localConnect true
    set insertFlop true
    set sigmaTFactor 2.0
    set minSeqBlocks 0
    set localCutOff 70
    set minInputs 1
    set minOutputs 1
    set minPathLen 0
    set maxPathLen 40
    set macroMinPathLen 2
    set macroMaxPathLen 20
    set pathLenCutOff 70
    set flopCutOff 70
    set flopDam 70
    set distThreshold 2
    set distMeanT 0.2
    set distMeanG 0.01
    set distFactor 1.3
    set flopInsertProb 0.1

    if { [info exists keys(-seed)] } {
        set seed $keys(-seed)
    }

    if { [info exists keys(-verbose)] } {
        set verbose $keys(-verbose)
    }

    if { [info exists keys(-allowLoops)] } {
        set allowLoops $keys(-allowLoops)
    }

    if { [info exists keys(-isAscend)] } {
        set isAscend $keys(-isAscend)
    }
    
    if { [info exists keys(-localConnect)] } {
        set localConnect $keys(-localConnect)
    }
    
    if { [info exists keys(-localCutOff)] } {
        set localCutOff $keys(-localCutOff)
    }

    if { [info exists keys(-sigmaTFactor)] } {
        set sigmaTFactor $keys(-sigmaTFactor)
    }

    if { [info exists keys(-minSeqBlocks)] } {
        set minSeqBlocks $keys(-minSeqBlocks)
    }
    
    if { [info exists keys(-minInputs)] } {
        set minInputs $keys(-minInputs)
    }

    if { [info exists keys(-minOutputs)] } {
        set minOutputs $keys(-minOutputs)
    }
    
    if { [info exists keys(-minPathLen)] } {
        set minPathLen $keys(-minPathLen)
    }
    
    if { [info exists keys(-maxPathLen)] } {
        set maxPathLen $keys(-maxPathLen)
    }
    
    if { [info exists keys(-macroMinPathLen)] } {
        set macroMinPathLen $keys(-macroMinPathLen)
    }

    if { [info exists keys(-macroMaxPathLen)] } {
        set macroMaxPathLen $keys(-macroMaxPathLen)
    }

    if { [info exists keys(-pathLenCutOff)] } {
        set pathLenCutOff $keys(-pathLenCutOff)
    }

    if { [info exists keys(-insertFlop)] } {
        set insertFlop $keys(-insertFlop)
    }

    if { [info exists keys(-flopCutOff)] } {
        set flopCutOff $keys(-flopCutOff)
    }
    
    if { [info exists keys(-flopDam)] } {
        set flopDam $keys(-flopDam)
    }

    if { [info exists keys(-distThreshold)] } {
        set distThreshold $keys(-distThreshold)
    }

    if { [info exists keys(-distMeanT)] } {
        set distMeanT $keys(-distMeanT)
    }

    if { [info exists keys(-distMeanG)] } {
        set distMeanG $keys(-distMeanG)
    }

    if { [info exists keys(-distFactor)] } {
        set distFactor $keys(-distFactor)
    }

    if { [info exists keys(-flopInsertProb)] } {
        set flopInsertProb $keys(-flopInsertProb)
    }
    
     if { [info exists keys(-defaultFlop)] } {
        set defaultFlop $keys(-defaultFlop)
        artnetgen_set_defaultFlop_cmd $defaultFlop
    }

    sta::check_positive_float "-sigmaTFactor" $sigmaTFactor
    sta::check_positive_float "-flopInsertProb" $flopInsertProb
    sta::check_positive_float "distMeanT" $distMeanT
    sta::check_positive_float "distMeanG" $distMeanG
    sta::check_positive_float "distFactor" $distFactor
    sta::check_positive_integer "-seed" $seed
    sta::check_positive_integer "-localCutOff" $localCutOff
    sta::check_positive_integer "-minInputs" $minInputs
    sta::check_positive_integer "-minOutput" $minOutputs
    sta::check_positive_integer "-maxPathLen" $maxPathLen
    sta::check_positive_integer "-minPathLen" $minPathLen
    sta::check_positive_integer "-macroMaxPathLen" $macroMaxPathLen
    sta::check_positive_integer "-macroMinPathLen" $macroMinPathLen
    sta::check_positive_integer "-pathLenCutOff" $pathLenCutOff
    sta::check_positive_integer "-flopCutOff" $flopCutOff
    sta::check_positive_integer "-flopDam" $flopDam
    sta::check_positive_integer "distThreshold" $distThreshold


    artnetgen_set_seed_cmd $seed
    artnetgen_set_verbose_cmd $verbose
    artnetgen_set_allowLoops_cmd $allowLoops
    artnetgen_set_isAscend_cmd $isAscend
    artnetgen_set_localConnect_cmd $localConnect
    artnetgen_set_localCutOff_cmd $localCutOff
    artnetgen_set_sigmaTFactor_cmd $sigmaTFactor
    artnetgen_set_minSeqBlocks $minSeqBlocks
    artnetgen_set_minInputs_cmd $minInputs
    artnetgen_set_minOutputs_cmd $minOutputs
    artnetgen_set_minPathLen_cmd $minPathLen
    artnetgen_set_maxPathLen_cmd $maxPathLen
    artnetgen_set_macroMinPathLen_cmd $macroMinPathLen
    artnetgen_set_macroMaxPathLen_cmd $macroMaxPathLen
    artnetgen_set_pathLenCutOff_cmd $pathLenCutOff
    artnetgen_set_insertFlop_cmd $insertFlop
    artnetgen_set_flopCutOff_cmd $flopCutOff
    artnetgen_set_flopDam_cmd $flopDam
    artnetgen_set_distThreshold_cmd $distThreshold
    artnetgen_set_distMeanT_cmd $distMeanT
    artnetgen_set_distMeanG_cmd $distMeanG
    artnetgen_set_distFactor_cmd $distFactor
    artnetgen_set_flopInsertProb_cmd $flopInsertProb

}

proc artnetgen_print_params { } {
    artnetgen_print_parameters_cmd
}



sta::define_cmd_args "artnetgen_init" {
    [-spec_file] \
    [-netlist] \
    [-dont_print_netlist] \
    [-clock_name] \
    [-is_clock] \
    [-is_reset] \
    [-print_netlist_info]
}

proc artnetgen_init { args } {
    sta::parse_key_args "artnetgen_init" args \
        keys { -spec_file \
               -netlist \
               -dont_print_netlist \
               -clock_name \
               -is_clock \
               -is_reset \
               -print_netlist_info } flags {}
    
    set is_clock true
    set is_reset true

    if { [info exists keys(-spec_file)] } {
        set spec_file $keys(-spec_file)
        artnetgen_set_spec_file_cmd $spec_file
    }
    
    if { [info exists keys(-netlist)] } {
        set file_name $keys(-netlist)
        artnetgen_set_netlist_cmd $file_name
    }
    
    if { [info exists keys(-dont_print_netlist)] } {
        set flag $keys(-dont_print_netlist)
        artnetgen_set_netlist_flag_cmd $flag
    }

    if { [info exists keys(-clock_name)] } {
        set clock_name $keys(-clock_name)
        artnetgen_set_clockName_cmd $clock_name
    }
    
    if { [info exists keys(-is_clock)] } {
        set is_clock $keys(-is_clock)
        artnetgen_set_is_clock_cmd $is_clock
    }
        
    if { [info exists keys(-is_reset)] } {
        set is_reset $keys(-is_reset)
        artnetgen_set_is_reset_cmd $is_reset
    }

    if { [info exists keys(-print_netlist_info)] } {
        set print_netlist_info $keys(-print_netlist_info)
        artnetgen_set_printNetlistInfo_cmd $print_netlist_info
    }

    artnetgen_init_cmd

}

proc artnetgen_run {  } {
    artnetgen_run_cmd
}

proc artnetgen_clear {  } {
    artnetgen_clear_cmd
}



sta::define_cmd_args "artnetgen_print_tree" {
    [-print_forrest]
}

proc artnetgen_print_tree { args } {
    sta::parse_key_args "artnetgen_print_tree" args \
        keys { -print_forrest } flags {}

    set print_forrest false

    if { [info exist keys(-print_forrest)] } {
        set print_forrest $keys(-print_forrest)
    }

    artnetgen_print_tree_cmd $print_forrest
}

proc artnetgen_check_floating { } {
    artnetgen_check_floating_cmd
}

proc artnetgen_print_masters { } {
    artnetgen_print_masters_cmd
}



sta::define_cmd_args "artnetgen_build_timing_path" {
    [-top_n] \
    [-is_max]
}

proc artnetgen_build_timing_path { args } {
    sta::parse_key_args "artnetgen_build_timing_path" args \
        keys { -top_n \
               -is_max } flags {}
    
    set top_n 1000
    set is_max 1

    if { [info exists keys(-top_n)] } {
        set top_n $keys(-top_n)
    }
    if { [info exists keys(-is_max)] } {
        set is_max $keys(-is_max)
    }

    artnetgen_build_timing_path_cmd $top_n $is_max
}



sta::define_cmd_args "artnetgen_write_gnl" {
    [-out_file]
}

proc artnetgen_write_gnl { args } {
    sta::parse_key_args "artnetgen_write_gnl" args \
        keys { -out_file } flags {}
    
    if { [info exists keys(-out_file)] } {
        set out_file $keys(-out_file)
    }
    artnetgen_write_gnl_cmd $out_file
}



sta::define_cmd_args "artnetgen_hnl2netlist" {
    [-in_file] \
    [-out_file] \
}

proc artnetgen_hnl2netlist { args } {
    sta::parse_key_args "artnetgen_hnl2netlist" args \
        keys { -in_file \
               -out_file } flags {}

    if { [info exists keys(-in_file)] } {
        set in_file $keys(-in_file)
    }
    if { [info exists keys(-out_file)] } {
        set out_file $keys(-out_file)
    }
    artnetgen_hnl2netlist_cmd $in_file $out_file
}

sta::define_cmd_args "artnetgen_reportCellDist" {
    [-out_file]
}

proc artnetgen_reportCellDist { args } {
    sta::parse_key_args "artnetgen_reportCellDist" args \
        keys { -out_file } flags {}

    if { [info exists keys(-out_file)] } {
        set out_file $keys(-out_file)
    }

    artnetgen_report_cell_dist_cmd $out_file
}

sta::define_cmd_args "artnetgen_get_edgebbox" {
    [-out_file]
}

proc artnetgen_get_edgebbox { args } {
    sta::parse_key_args "artnetgen_get_edgebbox" args \
        keys { -out_file } flags {}
    
    if { ![info exists keys(-out_file)] } {
    }

    set out_file $keys(-out_file)

    artnetgen_get_edge_bbox_cmd $out_file
}



sta::define_cmd_args "artnetgen_comp_real_ideal" {
    [-rent_constant] \
    [-rent_exponent] \
    [-num_insts] \
    [-out_file]
}

proc artnetgen_comp_edgebbox { args } {
    sta::parse_key_args "artnetgen_comp_real_ideal" args \
        keys { -rent_constant \
               -rent_exponent \
               -num_insts \
               -out_file } flags {}
    
    if { ![info exists keys(-rent_constant)] } {
    }
    if { ![info exists keys(-rent_eponent)] } {
    }
    if { ![info exists keys(-num_insts)] } {
    }
    if { ![info exists keys(-out_file)] } {
    }

    set rent_constant $keys(-out_file)
    set rent_exponent $keys(-out_file)
    set num_insts $keys(-out_file)
    set out_file $keys(-out_file)

    artnetgen_real_ideal_cmd $rent_constant $rent_exponent $num_insts $out_file
}



sta::define_cmd_args "artnetgen_write_verilog" { \
  [-port_prefix prefix] [-module_suffix suffix] [file]
}

proc artnetgen_write_verilog { args } {
  sta::parse_key_args "artnetgen_write_verilog" args \
    keys { -partitioning_id -port_prefix -module_suffix } flags { }

  sta::check_argc_eq1 "artnetgen_write_verilog" $args

  set port_prefix "partition_"
  if { [info exists keys(-port_prefix)] } {
    set port_prefix $keys(-port_prefix)
  }

  set module_suffix "_partition"
  if { [info exists keys(-module_suffix)] } {
    set module_suffix $keys(-module_suffix)
  }

  artnetgen_write_verilog_cmd $port_prefix $module_suffix $args
}




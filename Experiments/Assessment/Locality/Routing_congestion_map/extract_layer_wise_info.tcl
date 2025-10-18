# This script was written and developed by ABKGroup students at UCSD.
# However, the underlying commands and reports are copyrighted by Cadence.
# We thank Cadence for granting permission to share our research
# to help promote and foster the next generation of innovators.

proc get_track_count { box pitch offset numTrack is_horizontal } {
  if { $is_horizontal == 1 } {
    set loc1 [lindex $box 1]
    set loc2 [lindex $box 3]
  } else {
    set loc1 [lindex $box 0]
    set loc2 [lindex $box 2]
  }

  if { $offset > $loc2 } {
    return 0
  }

  set count1 [expr ceil(($loc1 - $offset) / $pitch)]
  set count2 [expr floor(($loc2 - $offset) / $pitch)]

  if { $count1 < 0 } {
    set count1 0
  }

  if { $count1 >= $numTrack } {
    return 0
  }
  if { $count2 < 0 } {
    set count2 0
  }

  if { $count2 >= $numTrack } {
    set count2 [expr $numTrack - 1]
  }

  set init_loc [expr $offset + $count1 * $pitch]
  set final_loc [expr $offset + $count2 * $pitch]
  
  if { $init_loc > $loc2 } {
    return 0
  }

  if { $init_loc == $final_loc } {
    return 1
  }

  set count [expr $count2 - $count1 + 1]
  return $count
}

proc get_wire_count { box layer pin_term_count } {
  set objs [dbQuery -areas $box -bbox_overlap -layers $layer -objType regular]
  set wire_count [llength  [dbget $objs.objType -v viaInst -e]]
  return [expr $wire_count + $pin_term_count]
}

proc get_layer_track_details { layer idx} {
  set track_ptr [dbget top.fplan.tracks.layers.name $layer -p2]
  set pitch [lindex [dbget $track_ptr.step] $idx]
  set offset [lindex [dbget $track_ptr.start] $idx]
  set numTrack [lindex [dbget $track_ptr.numTracks] $idx]
  return [list $pitch $offset $numTrack]
}

proc highlight_gcells {} {
  ## Write out the report congsetion file
  set congestion_rpt "sk_temp_congestion_report.rpt"
  dumpCongestArea $congestion_rpt -all
  set congfp [open $congestion_rpt "r"]
  set pattern {[0-9]+}
  while { [gets $congfp line] >= 0 } {
    # Ignore if line does not starts wiht (
    if { [regexp -nocase {^\(} $line] == 0 } {
      continue
    }
    set tokens [regexp -all -inline $pattern $line]
    set box [lrange $tokens 0 3]
    ## Divide all the elements of box by dbunit
    set box [lmap x $box {expr 1.0*$x / [dbget head.dbUnits]}]
    createMarker -bbox $box
  }
  close $congfp
  exec rm -rf $congestion_rpt
}

proc update_gcell { size offset } {
  set llx [dbget top.fplan.box_llx]
  set lly [dbget top.fplan.box_lly]
  set urx [dbget top.fplan.box_urx]
  set ury [dbget top.fplan.box_ury]

  set xoffset [expr $size * $offset]
  set yoffset [expr $size * $offset]
  set numx [ expr {int(ceil(($urx - $llx + $xoffset) / $size))}]
  set numy [ expr {int(ceil(($ury - $lly + $yoffset) / $size))}]
  set dbunit [dbget head.dbUnits]
  set xoffset [expr -1*int($xoffset * $dbunit)]
  set yoffset [expr -1*int($yoffset * $dbunit)]
  set size [expr int($size * $dbunit)]

  set top [dbget top.name]
  set fp [open "temp_gcell.def" "w"]
  puts $fp "VERSION 5.8 ;"
  puts $fp "DESIGN $top ;"
  puts $fp "UNITS DISTANCE MICRONS $dbunit ;"
  puts $fp "GCELLGRID X $xoffset DO $numx STEP $size ;"
  puts $fp "GCELLGRID Y $yoffset DO $numy STEP $size ;"
  puts $fp "END DESIGN"
  close $fp
  defIn "temp_gcell.def"
  exec rm -rf "temp_gcell.def"
}

proc get_layer_wise_details { output_file_name } {
  ## Write out the report congsetion file
  set congestion_rpt "sk_temp_congestion_report.rpt"
  dumpCongestArea $congestion_rpt -all
  
  ## Get the layer list
  set layer_list [dbget [dbget head.layers.type routing -p ].name]
  set layer_dir [dbget [dbget head.layers.type routing -p ].direction]

  ## Get the layer offset and pitch
  set layer_offset_list []
  set layer_pitch_list []
  set layer_track_count_list []

  foreach layer $layer_list {
    set layer_ptr [dbget head.layers.name $layer -p]
    set direction [dbget $layer_ptr.direction]
    if { $direction == "horizontal" } {
      set idx 0
    } else {
      set idx 1
    }
    set layer_details [get_layer_track_details $layer $idx]
    lappend layer_offset_list [lindex $layer_details 1]
    lappend layer_pitch_list [lindex $layer_details 0]
    lappend layer_track_count_list [lindex $layer_details 2]
    # lappend layer_dir $direction
  }

  ## Get design details
  set fp [open $output_file_name "w"]
  set congfp [open $congestion_rpt "r"]
  set heading_line "llx,lly,urx,ury,v_r,v_t,h_r,h_t"
  set idx 0
  while { $idx < [llength $layer_list] } {
    set layer [lindex $layer_list $idx]
    set suff [string index [lindex $layer_dir $idx] 0]
    append heading_line ",${layer}_${suff}_u,${layer}_${suff}_t"
    incr idx
  }
  puts $fp "$heading_line"

  set pattern {[0-9]+}
  ## Read line by line and get the congestion details
  while { [gets $congfp line] >= 0 } {
    # Ignore if line does not starts wiht (
    if { [regexp -nocase {^\(} $line] == 0 } {
      continue
    }
    set tokens [regexp -all -inline $pattern $line]
    set box [lrange $tokens 0 3]
    ## Divide all the elements of box by dbunit
    set box [lmap x $box {expr 1.0*$x / [dbget head.dbUnits]}]
    set new_line [join $box ","]
    set temp_line [join [lrange $tokens 4 end] ","]
    set new_line "${new_line},${temp_line}"
    
    ## Get pin count in the box
    set pin_terms [dbQuery -areas $box -bbox_overlap -objType instTerm]
    set pin_term_count_list []
    foreach layer $layer_list {
      set pin_term_count [llength [dbget $pin_terms.layer.name $layer -e]]
      lappend pin_term_count_list $pin_term_count
    }
    
    # puts $pin_term_count_list
    ## Get layer wise congestion details ##
    set idx 0
    while { $idx < [llength $layer_list] } {
      set layer [lindex $layer_list $idx]
      set pitch [lindex $layer_pitch_list $idx]
      set offset [lindex $layer_offset_list $idx]
      set numTrack [lindex $layer_track_count_list $idx]
      set is_horizontal [string match [lindex $layer_dir $idx] "Horizontal"]
      set track_count [get_track_count $box $pitch $offset $numTrack $is_horizontal]
      set wire_count [get_wire_count $box $layer [lindex $pin_term_count_list $idx]]
      append new_line ",$wire_count,$track_count"
      incr idx
    }
    puts $fp "$new_line"
  }

  close $fp
  close $congfp
  exec rm -rf $congestion_rpt
}

proc get_drc_rpt { file_name } {
  set fp [open $file_name "w"]
  foreach drc_ptr [dbget top.markers.type Geometry -p -e] {
    set bbox [concat {*}[dbget $drc_ptr.box]]
    set layer_type [dbget $drc_ptr.layer.type]
    set layer ""
    if { $layer_type == "routing" } {
      set layer [dbget $drc_ptr.layer.name]
    } elseif {$layer_type == "cut"} {
      set via_name [dbget $drc_ptr.layer.name]
      set top_layer [dbget [dbget head.vias.name ${via_name}* -p ].topLayer.name]
      set bot_layer [dbget [dbget head.vias.name ${via_name}* -p ].botLayer.name]
      set layer "${top_layer}_${bot_layer}"
    }
    set box [join $bbox ","]
    puts $fp "$box,$layer"
  }
  close $fp
}

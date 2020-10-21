source help.tcl
source gui_theme.tcl
package require Tk
package require Img   
encoding system utf-8 ;# For displaying non-latin ftp paths correctly

# --------------------------------Window-----------------------------------
wm title    . ftpImager
wm geometry . 800x600
wm minsize  . 710 380
wm iconbitmap . ftpicon.ico

# --------------------------------Menu---------------------------------
menu .menu
menu .menu.options -tearoff 0
menu .menu.help -tearoff 0

set onlyKnownExt 1
.menu.options add checkbutton -label "Only known extensions" -variable onlyKnownExt
.menu add cascade -menu .menu.options -label Options

.menu.help add command -label About -command showInfo
.menu add cascade -menu .menu.help -label Help

. config -menu .menu

# -----------------------------Info lines-------------------------------
pack [entry .path -textvar path      -font "Consolas 10" -bg [. cget -bg]           ] -side top    -fill x
pack [label .err  -textvar errorText -font "Consolas 11" -fg red          -anchor w ] -side bottom -fill x
proc errorColor color {.err config -fg $color}

# -----------------------Files and directories lists-----------------------
pack [frame .a            -relief solid -bd 1] -expand yes -fill both
pack [frame .a.left                          ] -side left  -fill y
pack [frame .a.left.right                    ] -side right -fill y

pack [listbox .a.left.dirs  -width 50 -relief sunken -bd 3 -highlightthickness 0] -side top    -fill y -expand yes 
pack [listbox .a.left.files -width 50 -relief sunken -bd 3 -highlightthickness 0] -side bottom -fill y -expand yes

pack [scrollbar .a.left.right.barDirs  -orient vertical -command {.a.left.dirs  yview} ] -side top    -fill y -expand yes 
pack [scrollbar .a.left.right.barFiles -orient vertical -command {.a.left.files yview} ] -side bottom -fill y -expand yes 

.a.left.dirs  config -yscrollcommand {.a.left.right.barDirs  set}
.a.left.files config -yscrollcommand {.a.left.right.barFiles set}

# -------------------------Things on the right-------------------------------
pack [frame  .a.right                         ]                     -expand yes -fill both -side right  -padx 0 -pady 0 -ipady 10 -ipadx 10
pack [canvas .a.right.view -bg $theme_color   ]                     -expand yes -fill both
pack [frame  .a.right.controls -height 20     ]                                 -fill x    -side bottom -pady 5
pack [label  .a.right.name -height 1 -font "Consolas 12" -textvar title]        -fill x    -side bottom 
# right.name becomes above the .right.controls

# --------------------------------Controls----------------------------------
set iback [image create photo -file images/back.bmp ]
set inext [image create photo -file images/next.bmp ]
set icw   [image create photo -file images/cw.bmp   ]
set iccw  [image create photo -file images/ccw.bmp  ]
set ifit  [image create photo -file images/fit.bmp  ]

# Change white -> transparent
foreach img [list $iback $inext $icw $iccw $ifit] {
	set r -1
	foreach row [$img data] {
		incr r
		set p -1
		foreach pixel $row {
			incr p
			if {$pixel == "#ffffff"} {$img transparency set $p $r 1}
		}
	}
}

pack [button .a.right.controls.goBack  -height 32 -width 40 -bg $theme_color  -image $iback -relief raised -bd 2 -command onGoBack ] -side left  -padx 10
pack [button .a.right.controls.goNext  -height 32 -width 40 -bg $theme_color  -image $inext -relief raised -bd 2 -command onGoNext ] -side left  -padx 10

pack [button .a.right.controls.fit     -height 32 -width 40 -bg $theme_color  -image $ifit  -relief raised -bd 2 -command onFit    ] -side right -padx 30

pack [button .a.right.controls.turnCCW -height 32 -width 40 -bg $theme_color  -image $iccw  -relief raised -bd 2 -command onTurnCCW] -side right -padx 10 -before .a.right.controls.fit
pack [button .a.right.controls.turnCW  -height 32 -width 40 -bg $theme_color  -image $icw   -relief raised -bd 2 -command onTurnCW ] -side right -padx 10 -before .a.right.controls.turnCCW

# ------------------------------------------Events---------------------------------------------

bind .path <Return> changePath

proc changePath {} {
	global errorText path hftp
	if [catch {ftp::Cd $hftp $path} ] {
		errorColor red
		set errorText "Invalid path"
	}
	updateLists
}

# Ctrl+X Ctrl+C Ctrl+V do not work when using non-English keyboard layout
# The following code is a solution for some Cyrillic layouts
event add <<Cut>>   <Control-division>
event add <<Copy>>  <Control-ntilde>
event add <<Paste>> <Control-igrave>

foreach btn [list goBack goNext turnCW turnCCW fit] {
	eval bind .a.right.controls.$btn <FocusIn>  "{.a.right.controls.$btn config -relief sunken}"
	eval bind .a.right.controls.$btn <FocusOut> "{.a.right.controls.$btn config -relief raised}"
}

proc onGoBack {} {
	set i [.a.left.files curselection]
	set l [.a.left.files size]
	if {$l == 0} {return}
	
	if {$i != ""} {.a.left.files selection clear $i}
	
	if {$i == "" || $i == 0} {
		set i [expr $l - 1]
	} else {
		incr i -1
	}
	
	.a.left.files selection set $i
	.a.left.files see $i
	
	onFileSelect
}

proc onGoNext {} {
	set i [.a.left.files curselection]
	set l [.a.left.files size]
	if {$l == 0} {return}
	
	if {$i != ""} {.a.left.files selection clear $i}
	
	if {$i == "" || $i == $l - 1} {
		set i 0
	} else {
		incr i
	}
	
	.a.left.files selection set $i
	.a.left.files see $i
	
	onFileSelect
}

proc onFit {} {
	fitToSize
	drawImg
}

bind .a.left.dirs  <Double-Button-1> onDirSelect
bind .a.left.dirs  <Return>          onDirSelect
bind .a.left.files <Double-Button-1> onFileSelect
bind .a.left.files <Return>          onFileSelect

wm protocol . WM_DELETE_WINDOW onWinClose

proc updateLists {} {
	global path dirs files
	
	.a.left.dirs  delete 0 end
	.a.left.files delete 0 end
	
	loadLists
	
	foreach item $dirs  {.a.left.dirs  insert end $item}
	foreach item $files {.a.left.files insert end $item}
}

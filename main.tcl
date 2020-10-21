package require ftp
load imgscale.dll
source ftp_settings.tcl
source gui.tcl

# console show

set downloadedImg 0
set finalImg 0

set startSuccess 1234
set path "/"
set dirs [list]
set files [list]
set extensions [list                 \
	jpeg jpg png gif bmp ico ps tiff \
	pcx ppm sgi sun tga xbm xpm      ]

set hftp [eval [concat ftp::Open \
	$FtpServer $FtpUserName $FtpPassword -port $FtpPort]]
	
if {$hftp == -1} {
	errorColor red
	set errorText [ concat                \
		Failed to connect to '$FtpServer' \
		on port '$FtpPort'                \
		with user name '$FtpUserName'     \
		and password '$FtpPassword'       ]
	set startSuccess 0
}

proc loadLists {} {
	global hftp path dirs files

	set all [ftp::List $hftp]
	
	set dirs [list]
	set files [list]
	
	foreach item $all {
		set parsed [concat {*}[split $item]]
		
		set name ""
		
		set len [llength $parsed]
		for {set i 8} {$i < $len} {incr i} {
			set name [concat $name [lindex $parsed $i]]
		}
		
		set type [lindex $parsed 0]
		
		if {[string match d* $type]} {
			lappend dirs $name
		} else {
			lappend files $name
		}
	}
}

if {$startSuccess == 1234} { updateLists }

proc onWinClose {} {
	global hftp
	ftp::Close $hftp
	destroy .
}

proc fitToSize {} {
	global downloadedImg finalImg
	if {$downloadedImg == 0} {return}
	
	set iw [image width $downloadedImg]
	set ih [image height $downloadedImg]
	set vw [winfo width .a.right.view]
	set vh [winfo height .a.right.view]
	
	if {$finalImg != 0} {image delete $finalImg}
	set finalImg [image create photo -width $vw -height $vh]
	
	set ratio [expr 1. * $iw / $ih]
	set w_by_h [expr int($vh * $ratio)]
	set h_by_w [expr int($vw / $ratio)]
	
	if {$w_by_h <= $vw} {
		set w $w_by_h
		set h $vh
	} else {
		set w $vw
		set h $h_by_w
	}
	
	imgscale::average $downloadedImg $w $h $finalImg
}

proc drawImg {} {
	global finalImg
	if {$finalImg == 0} {return}
	
	set vw [winfo width .a.right.view]
	set vh [winfo height .a.right.view]
	set x [expr int($vw / 2)]
	set y [expr int($vh / 2)]
	.a.right.view delete all
	.a.right.view create image $x $y -image $finalImg -anchor center
}

set downloadInProgress no
# Because if a few files are selected one by one too quikly,
# it will throw an error
proc onFileSelect {} {
	global hftp title onlyKnownExt errorText extensions downloadedImg \
		finalImg downloadInProgress
	
	if {$downloadInProgress} {return}

	# Get file name
	set i [lindex [.a.left.files curselection] 0]
	if {$i == ""} {return}
	set fn [.a.left.files get $i]
	
	# Check extension
	if {$onlyKnownExt} {
		set arr [split $fn .]
		set ext [string tolower [lindex $arr [expr [llength $arr]-1]]]
		if {!($ext in $extensions)} {
			errorColor red
			set errorText "Unknown extension"
			return
		}
	}
	
	# Download image file
	errorColor blue
	set errorText Downloading...
	set downloadInProgress yes
	
	set data 0
	ftp::Get $hftp $fn -variable data
	
	# Read file
	set errorText Processing...
	
	if {[catch {
		if {$downloadedImg != 0} {image delete $downloadedImg}
		set downloadedImg [image create photo -data $data]
	}]} {
		errorColor red
		set errorText "Invalid format"
		return
	}
	
	fitToSize
	
	drawImg
	
	set downloadInProgress no
	set title [concat ([image width $downloadedImg] x [image height $downloadedImg]) $fn]
	set errorText OK
}

proc onDirSelect {} {
	global path hftp errorText

	set i [lindex [.a.left.dirs curselection] 0]
	if {$i == ""} {return}
	
	set dn [.a.left.dirs get $i]
	
	errorColor blue
	set errorText Loading...
	
	if [catch {ftp::Cd $hftp $dn}]  {
		errorColor red
		set errorText "Failed to change directory"
		return
	}
	
	set path [ftp::Pwd $hftp]
	
	updateLists
	
	set errorText OK
}

# Turn clock-wise
proc onTurnCW {} {
	global downloadedImg
	if {$downloadedImg == 0} { return }
	
	set t [image create photo]		  
	cpp_rotateImage $downloadedImg $t 90
	image delete $downloadedImg
	set downloadedImg $t
	
	fitToSize
	
	drawImg
}

# Turn counter-clock-wise
proc onTurnCCW {} {
	global downloadedImg
	if {$downloadedImg == 0} { return }
	
	set t [image create photo]		  
	cpp_rotateImage $downloadedImg $t -90
	image delete $downloadedImg
	set downloadedImg $t
	
	fitToSize
	
	drawImg
}
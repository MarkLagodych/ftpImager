set infoText \
"_________________________________________
________________ftpImager________________
_______________version 1.1_______________
_______________Information_______________
_________________________________________

=============== Program =================
ftpImager views images on FTP servers:
jpeg, png, gif, bmp, ico, ps, tiff, pcx, 
ppm, sgi, sun, tga, xbm, xpm.

=============== Settings ================
See ftp_settings.tcl and gui_theme.tcl
files.

============ Double-clicks ==============
When you double-click in the directory 
list, you should wait 1 second until next
double-click.

============ Used libraries =============
tkImg
imgScale

"

proc showInfo {} {
	global infoText
	toplevel .i
	wm title .i "About ftpImager"
	wm geometry .i 440x430
	wm resizable .i no no
	wm iconbitmap .i ""
	wm attributes .i -topmost yes
	pack [text .i.text -font "Consolas 12"] -expand yes -fill both -padx 0 -pady 0
	.i.text insert end $infoText
}
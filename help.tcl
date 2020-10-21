set infoText \
"________________________________________
________________ftpImager________________
____________version 1.0.1 alpha__________
______________Information________________
_________________________________________

=============== Program =================
ftpImager views images on FTP servers:
jpeg, png, gif, bmp, ico, ps, tiff, pcx, 
ppm, sgi, sun, tga, xbm, xpm.

=============== Settings ================
Look at ftp_settings.tcl file.

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
	wm geometry .i 380x430
	wm resizable .i no no
	wm iconbitmap .i ""
	wm attributes .i -topmost yes
	pack [text .i.text -font "Consolas 12"] -expand yes -fill both -padx 0 -pady 0
	.i.text insert end $infoText
}
# FtpImager
This program views images on a certain FTP server.
The whole functionality is ONLY VIEWING, not moving/copying or whatever.


# Features
* Supported formats: jpeg, png, gif, bmp, ico, ps, tiff, pcx, ppm, sgi, sun, tga, xbm, xpm.
* Automatic extension checking. Can be disabled in Options menu.
* Image can be rotated and fited to the screen size.
* UTF-8 support.


# Notes
A server address, port and other settings can be specified in by modifying ```ftp_settings.tcl``` file.

The main color can be set by modifying ```gui_theme.tcl``` file.


# Building
**Prerequisites:** MS Windows, GCC, GNU Make, Active Tcl
```
make init
make
```

# Gallery
![](https://github.com/MarkLagodych/assets/blob/main/ftpImager/1.png?raw=true)
![](https://github.com/MarkLagodych/assets/blob/main/ftpImager/2.png?raw=true)
![](https://github.com/MarkLagodych/assets/blob/main/ftpImager/3.png?raw=true)
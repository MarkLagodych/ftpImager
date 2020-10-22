.PHONY: all clean init imgscale ftpImager

# ---------- TCL stuff -------------

# Path to Tcl *.lib files
tcl_libd  = "C:/mylib/ActiveTcl/lib"

# Path to Tcl *.h files
tcl_incd  = "C:/mylib/ActiveTcl/include"

# Tcl library
tcl_lib   = "tcl85"

# Tk library
tk_lib    = "tk85"

# ------------ Needed only if a standalone release is needed ---------------
# Note: in this case a ./lib directory will be created

# If set to "yes" then all Tcl/Tk will be copies to ./bin
copy_tcl_binaries = "yes"

# Tcl binaries directory
tcl_bind  = "C:/mylib/ActiveTcl/bin"

all: imgscale ftpImager
	cp *.tcl ./bin
	cp -r images ./bin/images
	cp -r Img ./bin/Img
ifeq ($(copy_tcl_binaries), "yes")
	cp $(tcl_bind)/*.dll ./bin
	cp -r $(tcl_libd)/* ./lib
	rm ./lib/*.lib
	rm ./lib/*.sh
	rm -r -f ./lib/ppm
endif
	echo "FtpImager project is built!"

init:
ifeq ($(wildcard ./obj),)
	mkdir ./obj
endif
ifeq ($(wildcard ./bin),)
	mkdir ./bin
endif
ifeq ($(wildcard ./lib),)
	mkdir ./lib
endif
	echo "Initialization completed!"
	

clean:
	rm -f -r ./obj/*
	rm -f -r ./bin/*
	rm -f -r ./lib/*

imgscale:
	gcc -c imgscale.c -o ./obj/imgscale.o -Wall -O2 -DBUILD_DLL -I$(tcl_incd)
	gcc -shared ./obj/imgscale.o -o ./bin/imgscale.dll -L$(tcl_libd) -l$(tcl_lib) -l$(tk_lib)
	echo "ImgScale library is compiled!"

ftpImager:
	g++ -c main.cpp -o ./obj/ftpImager.o -Wall -O2 -I$(tcl_incd)
	# -mwindows specifies a GUI application type that does not have a console window
	g++ ./obj/ftpImager.o -o ./bin/ftpImager.exe -L$(tcl_libd) -l$(tcl_lib) -l$(tk_lib) -mwindows
	echo "FtpImager program is compiled!"
#include <tcl.h>
#include <tk.h>
#include <fstream>
#include <string>
#include <windows.h>
//#include <iostream>

#define PIXEL(image, x, y, color)   \
    (image).pixelPtr [              \
    (y) * (image).pitch +           \
    (x) * (image).pixelSize +       \
    (image).offset[(color)]         \
]

#define IMAGE_CYCLE(_maxX, _maxY) \
    for (int y = 0; y <= (_maxY);  y++     ) {  \
    for (int x = 0; x <= (_maxX);  x++     ) {  \
    for (int color = 0; color < 3; color++ ) {

#define IMAGE_CYCLE_END }}}

#define SWAP(a, b)     \
    unsigned char tmp; \
    tmp = (a);         \
    (a) = (b);         \
    (b) = tmp;


using namespace std;

inline void ErrorBox(const char* msg) {
    MessageBox(0, msg, "ftpImager error", MB_ICONERROR);
}

int rotateImage(ClientData, Tcl_Interp*, int, Tcl_Obj* const*);

int main(int argc, char* argv[]) {
    Tcl_Interp *ti;
    ifstream file;

    // Load file
    file.open("main.tcl");
    if (!file) {
        ErrorBox("main.tcl: not found");
        return 1;
    }

    // Read file
    string line, text = "";
    while (getline(file, line))
        text += line + '\n';
    file.close();

    const char *script = text.c_str();

    // Init Tcl and Tk
    Tcl_FindExecutable(argv[0]);
    ti = Tcl_CreateInterp();
    if (Tcl_Init(ti) != TCL_OK) {
        ErrorBox("Tcl: can not initialize");
        return 2;
    }

    if (Tk_Init(ti) != TCL_OK) {
        ErrorBox("Tk: can not initialize");
        return 3;
    }

    Tcl_CreateObjCommand(ti, "cpp_rotateImage", rotateImage, NULL, NULL);

    // Execute Tcl
    Tcl_Eval(ti, script);

    // Run graphics
    Tk_MainLoop();

    return 0;
}

/* Usage: cpp_rotateImage source destination angle
   Copies destination to source width rotation in angle degrees
   angle = 90 or -90
   */
int rotateImage(ClientData clientData, Tcl_Interp *ti, int argc, Tcl_Obj* const argv[]) {
    if (argc != 4) {
        // => Wrong # args: should be cpp_rotateImage source destination
        Tcl_WrongNumArgs(ti, 0, NULL, "cpp_rotateImage source destination angle");
        return TCL_ERROR;
    }

    char
        *strSrc = Tcl_GetString(argv[1]),
        *strDst = Tcl_GetString(argv[2]);

    int angle;
    Tcl_GetIntFromObj(ti, argv[3], &angle);

    /* If it was called with 'cpp_rotateImage $img1 $img2 -90'
       Then now
       strSrc = result that '$img1' gives, for example, 'image9'
       strDst = result that '$img2' gives, for example, 'image7'
       angle  = -90 */

    Tk_PhotoHandle
        hSrc = Tk_FindPhoto(ti, strSrc),
        hDst = Tk_FindPhoto(ti, strDst);
    // hSrc and hDst are passed to image processing functions
    // like Tk_PhotoGetSize

    if (!hSrc && !hDst) {
        Tcl_AppendResult(ti, "Source or destination image does not exist");
        return TCL_ERROR;
    }

    int width, height;
    Tk_PhotoGetSize(hSrc, &width, &height);
    if (!width && !height) {
        Tcl_AppendResult(ti, "Source image has wrong size");
        return TCL_ERROR;
    }
    Tk_PhotoSetSize(ti, hDst, height, width);

    // Image blocks are structures that have raw image information
    // and a pointer to raw pixel data
    Tk_PhotoImageBlock src, dst;
    Tk_PhotoGetImage(hSrc, &src);

    // If image is not RGB or RGBA
    if (src.pixelSize != 3 && src.pixelSize != 4) {
        Tcl_AppendResult(ti, "Source image format is not supported");
        return TCL_ERROR;
    }

    Tk_PhotoBlank(hDst);

    // Dst image should be created
    // But its pixel array and other stuff not

    // Init image block
    dst.width = height; // because of rotation 90deg
    dst.height = width;
    dst.pixelSize = 3; // RGB
    dst.pitch = dst.width * dst.pixelSize;
    for (int i=0; i<3; i++)
        dst.offset[i] = i;
    // Create pixel array
    dst.pixelPtr = (unsigned char *) Tcl_Alloc(dst.pitch * dst.height);

    /*
    Raw pixel data looks like
    R G B [A] R G B [A] ...
    R G B [A] R G B [A] ...
    ...
    |<------------------->|
            .pitch
    (# of bytes per each row)

    .pixelPtr is a pointer to the R of the first pixel
    .pixelSize is # of bytes per pixel
    .width and .height are in pixels
    .offset is array of offsets of pixel's R, G, B[, A]

    So pixelPtr[5*pitch + 6*pixelSize + offset[0]]
    Returns R component of pixel in 5th row in 6th column
    (counting from 0)
    */

    // Mirror
    if (angle == -90) {
        IMAGE_CYCLE (src.width/2, src.height-1)
            SWAP (PIXEL(src, x, y, color), PIXEL(src, src.width-1-x, y, color));
        IMAGE_CYCLE_END
    }

    // Rotate
    IMAGE_CYCLE (src.width-1, src.height-1)
        PIXEL(dst, y, x, color) = PIXEL(src, x, y, color);
    IMAGE_CYCLE_END


    // Mirror
    if (angle == 90) {
        IMAGE_CYCLE (dst.width/2, dst.height-1)
            SWAP (PIXEL(dst, x, y, color), PIXEL(dst, dst.width-1-x, y, color));
        IMAGE_CYCLE_END
    }

    Tk_PhotoPutBlock(ti, hDst, &dst, 0, 0, dst.width, dst.height, TK_PHOTO_COMPOSITE_SET);

    return TCL_OK;
}


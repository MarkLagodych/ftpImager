/*
 * Image scaling using various algorithms.
 *
 * Copyright (C) 2011 Pat Thoyts <patthoyts@users.sourceforge.net>
 *
 * This can be built against 8.4 but will make use of the alpha channels and
 * extended error reporting if built with 8.5 stubs.
 *
 */

#include <tcl.h>
#include <tk.h>

#define CH(iptr,x, y, c) (iptr)->pixelPtr[(iptr)->pitch * (y) + (iptr)->pixelSize * x + (iptr)->offset[(c)]]
#define R(iptr,x,y) (iptr)->pixelPtr[(iptr)->pitch * (y) + (iptr)->pixelSize * x + (iptr)->offset[0]]
#define G(iptr,x,y) (iptr)->pixelPtr[(iptr)->pitch * (y) + (iptr)->pixelSize * x + (iptr)->offset[1]]
#define B(iptr,x,y) (iptr)->pixelPtr[(iptr)->pitch * (y) + (iptr)->pixelSize * x + (iptr)->offset[2]]
#define A(iptr,x,y) (iptr)->pixelPtr[(iptr)->pitch * (y) + (iptr)->pixelSize * x + (iptr)->offset[3]]
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

typedef int (*ScaleProc)(Tk_PhotoImageBlock *srcPtr, Tk_PhotoImageBlock *dstPtr, double newalpha);

#ifdef _WIN32
extern void __stdcall OutputDebugStringA(const char *);
static void _debug(const char *s) { OutputDebugStringA(s); }
#else
static void _debug(const char *s) { fprintf(stderr, "%s", s); }
#endif
static void debug(const char *template, ...)
{
    char sz[1024];
    va_list args;
    va_start(args, template);
    _vsnprintf(sz, 1023, template, args);
    sz[1023] = 0;
    va_end(args);
    _debug(sz);
}

/*
 * Use the nearest neighbour value.
 * When expanding a source image this simply fills new pixels from whatever source pixel
 * is closest. When shrinking better results may be obtained using an average of the
 * 2x2 cell neighbours. See Average.
 */

static int
Nearest(Tk_PhotoImageBlock *srcPtr, Tk_PhotoImageBlock *dstPtr, double newalpha)
{
    int y, x;
    double scalex, scaley;

    scalex = (double)srcPtr->width / (double)dstPtr->width;
    scaley = (double)srcPtr->height / (double)dstPtr->height;

    for (y = 0; y < dstPtr->height; ++y) {
        for (x = 0; x < dstPtr->width; ++x) {
            int sx = (int)(x * scalex);
            int sy = (int)(y * scaley);

            R(dstPtr, x, y) = R(srcPtr, sx, sy);
            G(dstPtr, x, y) = G(srcPtr, sx, sy);
            B(dstPtr, x, y) = B(srcPtr, sx, sy);
            if (dstPtr->pixelSize == 4)
                A(dstPtr, x, y) = (unsigned char)(((srcPtr->pixelSize == 4) ? A(srcPtr, sx, sy) : 255) * newalpha);
        }
    }
    return TCL_OK;
}

/*
 * This interpolates using the simple average of the 4 nearest neighbours.
 */
static int
Average(Tk_PhotoImageBlock *srcPtr, Tk_PhotoImageBlock *dstPtr, double newalpha)
{
    int y, x;
    double scalex = (double)srcPtr->width / (double)dstPtr->width;
    double scaley = (double)srcPtr->height / (double)dstPtr->height;
    double scalex2 = scalex / 2.0;
    double scaley2 = scaley / 2.0;

    for (y = 0; y < dstPtr->height; ++y) {
        for (x = 0; x < dstPtr->width; ++x) {
            int cR = 0, cG = 0, cB = 0, cA = 0, sx2, sy2;
            int sx = (int)(x * scalex);
            int sy = (int)(y * scaley);
            sx2 = (int)(sx + scalex2);
            sx2 = MIN(sx2, srcPtr->width - 1);
            sy2 = (int)(sy + scaley2);
            sy2 = MIN(sy2, srcPtr->height - 1);

            cR += R(srcPtr, sx, sy2);
            cG += G(srcPtr, sx, sy2);
            cB += B(srcPtr, sx, sy2);
            cR += R(srcPtr, sx, sy);
            cG += G(srcPtr, sx, sy);
            cB += B(srcPtr, sx, sy);
            cR += R(srcPtr, sx2, sy2);
            cG += G(srcPtr, sx2, sy2);
            cB += B(srcPtr, sx2, sy2);
            cR += R(srcPtr, sx2, sy);
            cG += G(srcPtr, sx2, sy);
            cB += B(srcPtr, sx2, sy);
            if (srcPtr->pixelSize == 4) {
                cA += A(srcPtr, sx, sy2);
                cA += A(srcPtr, sx, sy);
                cA += A(srcPtr, sx2, sy2);
                cA += A(srcPtr, sx2, sy);
            }

            R(dstPtr, x, y) = cR / 4;
            G(dstPtr, x, y) = cG / 4;
            B(dstPtr, x, y) = cB / 4;
            if (dstPtr->pixelSize == 4) {
                A(dstPtr, x, y) = (unsigned char)((cA / 4) * newalpha);
            }
        }
    }
    return TCL_OK;
}

/* bilinear interpolation - code derived from the 'crimp' implementation */
static int
Bilinear(Tk_PhotoImageBlock *srcPtr, Tk_PhotoImageBlock *dstPtr, double newalpha)
{
    int y, x, c;
    double scalex = (double)srcPtr->width / (double)dstPtr->width;
    double scaley = (double)srcPtr->height / (double)dstPtr->height;

    for (y = 0; y < dstPtr->height; ++y) {
        for (x = 0; x < dstPtr->width; ++x) {
            double xf = (x * scalex);
            double yf = (y * scaley);
            int ixw = (int)xf;
            int iyw = (int)yf;
            xf -= (double)ixw;
            yf -= (double)iyw;

            for (c = 0; c < dstPtr->pixelSize; ++c) {
                double val = 0;
                int ix, iy;
                for (iy = MAX(iyw, 0); iy < MIN(iyw + 2, srcPtr->height); ++iy) {
                    yf = 1 - yf;
                    for (ix = MAX(ixw, 0); ix < MIN(ixw + 2, srcPtr->width); ++ix) {
                        xf = 1 - xf;
                        val += CH(srcPtr, ix, iy, c) * yf * xf;
                    }
                }
                if (c == dstPtr->offset[3]) val *= newalpha;
                CH(dstPtr, x, y, c) = (unsigned char)val;
            }
        }
    }
    return TCL_OK;
}

/*
 * Scale an image using grid sampling with a passed in interpolation function.
 *
 * source     - a tk photo created with "image create photo"
 * new_width  - the desired width of the new image in pixels
 * new_height - the desired height of the new image in pixels
 * destination- a destination image created with "image create photo"
 * alpha      - alpha to apply to the image from 0 (transparent) to 1.0 (opaque)
 */

static int
ScaleCmd(void *clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    char *srcName, *destName;
    Tk_PhotoImageBlock srcBlock, destBlock;
    Tk_PhotoHandle srcImage, destImage;
    ScaleProc scaleProc = (ScaleProc)clientData;
    double newalpha;
    int width, height, width_dst, height_dst, r = TCL_OK;

    if (objc != 5 && objc != 6) {
        Tcl_WrongNumArgs(interp, 1, objv, "source new_width new_height destination ?alpha?");
        return TCL_ERROR;
    }
    srcName = Tcl_GetString(objv[1]);
    if (Tcl_GetIntFromObj(interp, objv[2], &width_dst))
        return TCL_ERROR;
    if (Tcl_GetIntFromObj(interp, objv[3], &height_dst))
        return TCL_ERROR;
    destName = Tcl_GetString(objv[4]);
    if (objc == 6) {
        if (Tcl_GetDoubleFromObj(interp, objv[5], &newalpha))
            return TCL_ERROR;
    } else {
        newalpha = 1.0;
    }
    if (newalpha > 1.0) {
        newalpha = 1.0;
    }

    srcImage = Tk_FindPhoto(interp, srcName);
    if (!srcImage) {
        Tcl_AppendResult(interp, "image \"", srcName, "\" does not exist", NULL);
        return TCL_ERROR;
    }

    Tk_PhotoGetSize(srcImage, &width, &height);
    Tk_PhotoGetImage(srcImage, &srcBlock);
    if (srcBlock.pixelSize != 4 && srcBlock.pixelSize != 3) {
        Tcl_AppendResult(interp, "invalid image format: image pixel size is unsupported", NULL);
        return TCL_ERROR;
    }

    destImage = Tk_FindPhoto(interp, destName);
    if (!destImage) {
        Tcl_AppendResult(interp, "image \"", destName, "\" does not exist", NULL);
        return TCL_ERROR;
    }

    Tk_PhotoBlank(destImage);
#if 10 * TK_MAJOR_VERSION + TK_MINOR_VERSION > 84
    Tk_PhotoSetSize(interp, destImage, width_dst, height_dst);
#else
    Tk_PhotoSetSize(destImage, width_dst, height_dst);
#endif

    destBlock.width = width_dst;
    destBlock.height = height_dst;
    destBlock.pixelSize = 4;
    destBlock.pitch = destBlock.width * destBlock.pixelSize;
    destBlock.offset[0] = 0;
    destBlock.offset[1] = 1;
    destBlock.offset[2] = 2;
    destBlock.offset[3] = 3;
    destBlock.pixelPtr = (unsigned char *) Tcl_Alloc(destBlock.pitch * destBlock.height);

    r = scaleProc(&srcBlock, &destBlock, newalpha);
    if (TCL_OK == r) {
#if 10 * TK_MAJOR_VERSION + TK_MINOR_VERSION > 84
        r = Tk_PhotoPutBlock(interp, destImage, &destBlock, 0, 0,
                             destBlock.width, destBlock.height, TK_PHOTO_COMPOSITE_SET);
#else
        Tk_PhotoPutBlock(destImage, &destBlock, 0, 0,
                         destBlock.width, destBlock.height, TK_PHOTO_COMPOSITE_SET);
#endif
    }
    return r;
}

#ifndef TCL_VERSION_MINIMUM
#define TCL_VERSION_MINIMUM "8.4"
#endif

int DLLEXPORT
Imgscale_Init(Tcl_Interp *interp)
{
    if (Tcl_InitStubs(interp, TCL_VERSION_MINIMUM, 0) == NULL) {
        return TCL_ERROR;
    }
    if (Tcl_PkgRequire(interp, "Tcl", TCL_VERSION_MINIMUM, 0) == NULL) {
        return TCL_ERROR;
    }
    if (Tk_InitStubs(interp, TCL_VERSION_MINIMUM, 0) == NULL) {
        return TCL_ERROR;
    }
    if (Tcl_PkgRequire(interp, "Tk", TCL_VERSION_MINIMUM, 0) == NULL) {
        return TCL_ERROR;
    }

    Tcl_CreateObjCommand(interp, "imgscale::nearest", ScaleCmd, (ClientData)Nearest, NULL);
    Tcl_CreateObjCommand(interp, "imgscale::average", ScaleCmd, (ClientData)Average, NULL);
    Tcl_CreateObjCommand(interp, "imgscale::bilinear", ScaleCmd, (ClientData)Bilinear, NULL);
    return Tcl_PkgProvide(interp, "imgscale", "1.0");
}

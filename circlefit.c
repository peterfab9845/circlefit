#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <png.h>

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color;
typedef color pixel;

typedef struct {
    int x;
    int y;
    int r;
} circle;

png_image orig;
pixel *origbuf;
pixel *outbuf;

color getpixel(int x, int y) {
    // bytes per memory row = components per row * bytes per component
    int stride = PNG_IMAGE_ROW_STRIDE(orig) * PNG_IMAGE_PIXEL_COMPONENT_SIZE(orig.format);
    // pixels per memory row = bytes per row / bytes per pixel
    int pitch = stride/PNG_IMAGE_PIXEL_SIZE(orig.format);
    return origbuf[y*pitch + x];
}

void putpixel(int x, int y, color c) {
    // for the output, pixels per memory row is known to be image width
    int pitch = orig.width;
    outbuf[y*pitch + x] = c;
}

void xline(int xa, int xb, int y, color c) {
    if (xa > xb)
        return;
    for (int i = xa; i <= xb; i++) {
        putpixel(i, y, c);
    }
}

// draw points in all 8 symmetric octants of a circle
void draw_circle_octant_points(int cx, int cy, int x, int y, color c) {
    // quadrants of circle
    putpixel(cx + x, cy + y, c);
    putpixel(cx + x, cy - y, c);
    putpixel(cx - x, cy + y, c);
    putpixel(cx - x, cy - y, c);
    if (x != y) {
        // also do octants by swapping x and y
        putpixel(cx + y, cy + x, c);
        putpixel(cx + y, cy - x, c);
        putpixel(cx - y, cy + x, c);
        putpixel(cx - y, cy - x, c);
    }
}

// fill a circle using octants
void fill_circle_octant_points(int cx, int cy, int x, int y, color c) {
    // quadrants of circle
    xline(cx - x, cx + x, cy + y, c);
    xline(cx - x, cx + x, cy - y, c);
    if (x != y) {
        // also do octants by swapping x and y
        xline(cx - y, cx + y, cy + x, c);
        xline(cx - y, cx + y, cy - x, c);
    }

}

// Bresenham Circle Drawing Algorithm
// https://funloop.org/post/2021-03-15-bresenham-circle-drawing-algorithm.html
void draw_circle(int fill, circle cir, color col) {
    // Calculation coordinates are based on (0, 0) at center, math polarity.
    // Start in standard position.
    int x = cir.r;
    int y = 0;

    // F = distance from true circle
    int F = 1 - cir.r; // approx for (r - 0.5, 1)
    // dN and dNW = how much F will change when going the respective direction
    int dN = 3;
    int dNW = 5 - (2 * cir.r);

    // first point
    if (fill)
        fill_circle_octant_points(cir.x, cir.y, x, y, col);
    else
        draw_circle_octant_points(cir.x, cir.y, x, y, col);

    while (x > y) {
        if (F < 0) {
            // Northwest would go too far inside the circle, go north instead. X remains the same.
            // Update F: increases by dN
            F += dN;
            // Update dN and dNW
            dN += 2;
            dNW += 2;
        } else {
            // Go northwest by default.
            x--;
            // Update F: increases by dNW
            F += dNW;
            // Update dN and dNW
            dN += 2;
            dNW += 4;
        }
        y++;
        if (fill)
            fill_circle_octant_points(cir.x, cir.y, x, y, col);
        else
            draw_circle_octant_points(cir.x, cir.y, x, y, col);
    }
}

void read_image(void) {
    orig.version = PNG_IMAGE_VERSION;
    orig.opaque = NULL;

    png_image_begin_read_from_stdio(&orig, stdin);

    orig.format = PNG_FORMAT_RGB;

    origbuf = malloc(PNG_IMAGE_SIZE(orig));
    png_image_finish_read(&orig, NULL, origbuf, 0, NULL);
}

int main(int argc, char *argv[]) {
    read_image();
    outbuf = calloc(orig.width * orig.height, sizeof(pixel));

    /*
    for (int x = 0; x < orig.width; x++) {
        for (int y = 0; y < orig.height; y++) {
            color c = getpixel(x, y);
            c.r = 255 - c.r;
            c.g = 255 - c.g;
            c.b = 255 - c.b;
            putpixel(x, y, c);
        }
    }
    */

    circle cir;
    cir.x = 500;
    cir.y = 500;
    cir.r = 20;

    color col;
    col.r = 0xff;
    col.g = 0x00;
    col.b = 0x00;

    draw_circle(1, cir, col);

    col.g = 0xff;
    draw_circle(0, cir, col);

    fwrite(outbuf, sizeof(pixel), orig.width * orig.height, stdout);

    free(origbuf);
    png_image_free(&orig);
    free(outbuf);
}


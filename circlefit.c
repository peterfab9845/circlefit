#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <png.h>

#define SQUARE(x) ((x) * (x))

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color;
typedef color pixel;

typedef struct circle_t {
    int x;
    int y;
    int r;
} circle;

typedef struct {
    union {
        struct circle_t; // MS extension
        circle cir;
    };
    bool alive;
} box;

int boxes_size;
int nboxes;
box *boxes;

png_image orig;
pixel *origbuf;
pixel *outbuf;

color getpixel(int x, int y) {
    // bytes per memory row = components per row * bytes per component
    int stride = PNG_IMAGE_ROW_STRIDE(orig) *
        PNG_IMAGE_PIXEL_COMPONENT_SIZE(orig.format);
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
// CAUTION: No bounds checking
void draw_circle(bool fill, circle cir, color col) {
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
        if (F <= 0) {
            // Northwest would go too far inside the circle, go north instead.
            // X remains the same.
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

void draw_box(box *b, color fill, color edge) {
    draw_circle(true, b->cir, fill);
    draw_circle(false, b->cir, edge);
}

// Will these two circles collide if both grow by pad?
// Based on XScreenSaver boxfit by jwz
bool circles_collide(circle *a, circle *b, int pad) {
    // squared distance between circle centers
    int centers = SQUARE(b->x - a->x) + SQUARE(b->y - a->y);
    // squared sum of radii
    int radii = SQUARE(a->r + b->r + pad);
    return (centers < radii);
}

// Will this box be legal if it grows by pad?
// Based on XScreenSaver boxfit by jwz
bool box_legal(box *a, int pad) {
    if (a->x - a->r - pad < 0 ||
        a->y - a->r - pad < 0 ||
        (unsigned int)(a->x + a->r + pad) >= orig.width || // TODO an extra pad?
        (unsigned int)(a->y + a->r + pad) >= orig.height   // ^
    ) {
        return false;
    }

    for (int i = 0; i < nboxes; i++) {
        box *b = &boxes[i];
        if ((a != b) && circles_collide(&a->cir, &b->cir, pad)) {
            return false;
        }
    }
    return true;
}

// Read a PNG format image from stdin to buf
void read_image(png_image *image, pixel **buf) {
    image->version = PNG_IMAGE_VERSION;
    image->opaque = NULL;

    png_image_begin_read_from_stdio(image, stdin);

    image->format = PNG_FORMAT_RGB;

    *buf = malloc(PNG_IMAGE_SIZE(*image));
    png_image_finish_read(image, NULL, *buf, 0, NULL);
}

int main(void) {

    // TODO make these command-line options
    int maxalive = 100;   // max number of live boxes at a time
    int maxcount = 65535; // max total number of boxes
    int padding = 2;      // padding between boxes and on edges
    int growby = 1;       // amount to increase radius each iteration
    int minradius = 5;    // minimum radius of a circle
    color edgecol = {0x30, 0x30, 0x30}; // border color of boxes

    read_image(&orig, &origbuf);
    outbuf = calloc(orig.width * orig.height, sizeof(pixel));

    srand(time(NULL));

    // Circle generation algorithm
    // Based on XScreenSaver boxfit by jwz
    boxes_size = 2 * maxalive;
    boxes = calloc(boxes_size, sizeof(*boxes));
    nboxes = 0;

    int nalive = 0;
    bool finished = false;

    while (!finished) {
        // grow boxes if possible
        for (int i = 0; i < nboxes; i++) {
            box *b = &boxes[i];

            if (!b->alive) {
                // don't keep growing, it's already dead
            } else if (!box_legal(b, growby + padding)) {
                // can't grow anymore, make it dead
                b->alive = false;
                nalive--;
            } else {
                // grow the box
                b->r += growby;
            }
        }

        // add new boxes if needed
        while (nalive < maxalive) {
            if (boxes_size <= nboxes) {
                // need to reallocate
                boxes_size = (1.5 * boxes_size) + nboxes;
                boxes = realloc(boxes, boxes_size * sizeof(*boxes));
                if (!boxes) {
                    fprintf(stderr, "Failed to allocate memory for %d boxes\n",
                            boxes_size);
                    exit(ENOMEM);
                }
            }

            // try to add a new box 100 times
            box *b = &boxes[nboxes];
            for (int i = 0; i < 100; i++) {
                b->x = padding + (rand() % (orig.width - 2*padding));
                b->y = padding + (rand() % (orig.height - 2*padding));
                b->r = minradius;

                if (box_legal(b, padding)) {
                    // successfully found a spot
                    b->alive = true;
                    nboxes++;
                    nalive++;
                    break;
                }
            }
            if (!b->alive || nboxes >= maxcount) {
                // unable to find a new box to add, or reached max
                finished = true;
                break;
            }
        }
    }

    // draw boxes
    for (int i = 0; i < nboxes; i++) {
        box *b = &boxes[i];
        draw_box(b, getpixel(b->x, b->y), edgecol);
    }

    // TODO libpng output to file (optionally)
    fwrite(outbuf, sizeof(pixel), orig.width * orig.height, stdout);

    free(origbuf);
    png_image_free(&orig);
    free(outbuf);

    return 0;
}


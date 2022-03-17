#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <getopt.h>
#include <string.h>
#include <png.h>
#include <libnsbmp.h>
#include <sys/stat.h>

#define BMP_BYTES_PER_PIXEL (4)
#define SQUARE(x) ((x) * (x))

typedef struct color_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color;
typedef color pixel;

typedef struct {
    union {
        struct color_t; // MS extension
        pixel pix3;
    };
    uint8_t a;
} pixel4;

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

typedef enum { PNG, BMP } image_format_t;
image_format_t img_format;

int nboxes;
box *boxes;

int img_width;
int img_height;

png_image orig_png;
pixel *orig_png_buf;

void *bmp_cb_create(int width, int height, unsigned int flags);
void bmp_cb_destroy(void *bitmap);
unsigned char *bmp_cb_get_buffer(void *bitmap);
size_t bmp_cb_get_bpp(void *bitmap);

bmp_image orig_bmp;
bmp_bitmap_callback_vt bmp_callbacks = {
    bmp_cb_create,
    bmp_cb_destroy,
    bmp_cb_get_buffer,
    bmp_cb_get_bpp
};

pixel *outbuf;

color getpixel(int x, int y) {
    if (img_format == PNG) {
        // bytes per memory row = components per row * bytes per component
        int stride = PNG_IMAGE_ROW_STRIDE(orig_png) *
            PNG_IMAGE_PIXEL_COMPONENT_SIZE(orig_png.format);
        // pixels per memory row = bytes per row / bytes per pixel
        int pitch = stride/PNG_IMAGE_PIXEL_SIZE(orig_png.format);
        return orig_png_buf[y*pitch + x];
    } else if (img_format == BMP) {
        // pixels per memory row = image width
        int pitch = orig_bmp.width;
        return ((pixel4 *)(orig_bmp.bitmap))[y*pitch + x].pix3;
    }
    return (color){0x00, 0x00, 0x00};
}

void putpixel(int x, int y, color c) {
    // for the output, pixels per memory row is known to be image width
    int pitch = img_width;
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

// Draw a box with fill and edge colors
void draw_box(box *b, color fill, color edge) {
    draw_circle(true, b->cir, fill);
    draw_circle(false, b->cir, edge);
}

// Read a PNG format image from stdin into buf
void read_png_stdio(png_image *image, pixel **buf) {
    image->version = PNG_IMAGE_VERSION;
    image->opaque = NULL;

    if (!png_image_begin_read_from_stdio(image, stdin)) {
        // TODO
    }

    image->format = PNG_FORMAT_RGB;

    *buf = calloc(1, PNG_IMAGE_SIZE(*image));
    if (!*buf) {
        // TODO
    }

    if (!png_image_finish_read(image, NULL, *buf, 0, NULL)) {
        // TODO
    }
}

// Read a PNG format image from path into buf
void read_png_file(png_image *image, pixel **buf, char *path) {
    image->version = PNG_IMAGE_VERSION;
    image->opaque = NULL;

    if (!png_image_begin_read_from_file(image, path)) {
        // TODO
    }

    image->format = PNG_FORMAT_RGB;

    *buf = calloc(1, PNG_IMAGE_SIZE(*image));
    if (!*buf) {
        // TODO
    }

    if (!png_image_finish_read(image, NULL, *buf, 0, NULL)) {
        // TODO
    }
}

// Write a PNG format image to stdout from buf
void write_png_stdio(pixel **buf, int width, int height) {
    // TODO
}

// Write a PNG format image to path from buf
void write_png_file(pixel **buf, int width, int height, char *path) {
    // TODO
}

// Read a file into newly allocated memory
// Sets size to the file size
char *read_file(char *path, size_t *size) {
    FILE *fd = fopen(path, "rb");
    if (!fd) {
        fprintf(stderr, "Failed to open file %s\n", path);
        exit(EXIT_FAILURE);
    }

    struct stat sb;
    if (stat(path, &sb)) {
        fprintf(stderr, "Failed to stat file %s\n", path);
        exit(EXIT_FAILURE);
    }
    *size = sb.st_size;

    void *buffer = malloc(*size);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate %zu bytes\n", *size);
        exit(EXIT_FAILURE);
    }

    size_t nread = fread(buffer, 1, *size, fd);
    if (nread != *size) {
        fprintf(stderr, "Unable to read %zu bytes, got %zu\n", *size, nread);
        exit(EXIT_FAILURE);
    }

    fclose(fd);
    return buffer;
}

// Read a BMP file from stdin into newly allocated memory
// Sets size to the file size
char *read_bmp_stdio(size_t *size) {
    // Read the first 2 bytes and check the signature
    uint16_t signature;
    if (fread(&signature, sizeof(uint16_t), 1, stdin) != 1) {
        fprintf(stderr, "Failed to read BMP signature from stdin\n");
        exit(EXIT_FAILURE);
    }
    if ((signature        & 0xFF) != 'B' || ((signature >> 8) & 0xFF) != 'M') {
        fprintf(stderr, "BMP signature does not match\n");
        exit(EXIT_FAILURE);
    }

    // Read the next 4 bytes for the file size
    uint32_t filesize;
    if (fread(&filesize, sizeof(uint32_t), 1, stdin) != 1) {
        fprintf(stderr, "Failed to read BMP filesize from stdin\n");
        exit(EXIT_FAILURE);
    }
    *size = filesize;

    // allocate memory for the rest of the file
    char *buffer = malloc(*size);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate %zu bytes\n", *size);
        exit(EXIT_FAILURE);
    }

    // fill in what we already read
    buffer[0] = 'B';
    buffer[1] = 'M';
    buffer[2] = (filesize >> 24) & 0xFF;
    buffer[3] = (filesize >> 16) & 0xFF;
    buffer[4] = (filesize >> 8) & 0xFF;
    buffer[5] = filesize & 0xFF;

    // read the rest of the file
    size_t nread = fread(buffer + 6, 1, filesize - 6, stdin);
    if (nread != filesize - 6) {
        fprintf(stderr, "Unable to read %u remaining bytes, got %zu\n",
                filesize - 6, nread);
        exit(EXIT_FAILURE);
    }

    return buffer;
}

// BMP reading callback functions
// Based on libnsbmp decode_bmp example
void *bmp_cb_create(int width, int height, unsigned int flags) {
    // BMP_NEW and BMP_OPAQUE flags unused
    if (flags & BMP_CLEAR_MEMORY) {
        return calloc(width * height, BMP_BYTES_PER_PIXEL);
    } else {
        return malloc(width * height * BMP_BYTES_PER_PIXEL);
    }
}

void bmp_cb_destroy(void *bitmap) {
    free(bitmap);
}

unsigned char *bmp_cb_get_buffer(void *bitmap) {
    return bitmap;
}

size_t bmp_cb_get_bpp(void *bitmap) {
    (void) bitmap; // unused
    return BMP_BYTES_PER_PIXEL;
}

// Decode a BMP format image stored in filebuf
// TODO check error handling
int decode_bmp(bmp_image *image, bmp_bitmap_callback_vt *callbacks,
        void *filebuf, size_t size) {
    bmp_create(image, callbacks);

    bmp_result result = bmp_analyse(image, size, filebuf);
    if (result != BMP_OK) {
        return -1;
    }

    result = bmp_decode(image);
    if (result != BMP_OK) {
        return -1;
    }

    return 0;
}

// Will these two circles collide if one grows by incr?
// Based on XScreenSaver boxfit by jwz
bool circles_collide(circle *a, circle *b, int incr) {
    // squared distance between circle centers
    int centers = SQUARE(b->x - a->x) + SQUARE(b->y - a->y);
    // squared sum of radii
    int radii = SQUARE(a->r + b->r + incr);
    return (centers < radii);
}

bool boxes_collide(box *a, box *b, int incr) {
    return circles_collide(&a->cir, &b->cir, incr);
}

// Will this box be in bounds if it grows by incr?
bool box_in_bounds(box *a, int incr) {
    if (a->x - a->r - incr < 0 ||
        a->y - a->r - incr < 0 ||
        a->x + a->r + incr >= img_width ||
        a->y + a->r + incr >= img_height
    ) {
        return false;
    }
    return true;
}

// Will this box be in bounds with no collisions if it grows by incr?
bool box_legal(box *a, int incr) {
    if (!box_in_bounds(a, incr)) {
        return false;
    }

    for (int i = 0; i < nboxes; i++) {
        box *b = &boxes[i];
        if ((a != b) && boxes_collide(a, b, incr)) {
            return false;
        }
    }

    return true;
}

void usage(void) {
    fprintf(stderr, "Usage: circlefit [OPTION]...\n\
Generate circles colored by the given image.\n\n\
Required arguments apply to both long and short options.\n\
  -h, --help                  display this help text and exit\n\
  -a, --max-alive=INT         maximum number of circles alive concurrently;\n\
                                default 100, must be at least 1\n\
  -t, --max-total=INT         maximum total number of circles;\n\
                                default 65535, must be at least 0\n\
  -r, --min-radius=INT        minimum radius of circle to create;\n\
                                default 5, must be at least 1\n\
  -p, --padding=INT           padding between circles and on edges;\n\
                                default 2, must be at least 0\n\
  -g, --grow-by=INT           amount to increase each circle's radius per tick;\n\
                                default 1, must be at least 1\n\
  -c, --edge-color=RRGGBB     hex color of the edge of circles; default 303030\n\
  -i, --input-file=STRING     input filename, stdin if not provided\n\
  -f, --input-format=STRING   input format, guessed from filename if possible;\n\
                                default 'bmp'. 'bmp' and 'png' supported\n\
  -o, --output-file=STRING    output filename, stdout if not provided\n\
  -F, --output-format=STRING  output format, guessed from filename if possible;\n\
                                default is input format.\n\
                                'bmp', 'png' and 'raw' (24bpp RGB) supported\n");
}

int main(int argc, char *argv[]) {

    int max_alive = 100;   // max number of live boxes at a time
    int max_total = 65535; // max total number of boxes
    int min_radius = 5;    // minimum radius of a circle
    int padding = 2;       // padding between boxes and on edges
    int grow_by = 1;       // amount to increase radius each iteration
    color edge_color = {0x30, 0x30, 0x30}; // border color of boxes

    char edge_color_str[8] = {0};
    char input_filename[256] = {0};
    char input_format[8] = {0};
    char output_filename[256] = {0};
    char output_format[8] = {0};

    img_format = BMP;
    bool use_stdin = true;

    // get command-line options
    int rc;
    int option_index = 0;
    char *options = "ha:t:r:p:g:c:i:f:o:F:";
    struct option long_options[] = {
        {"help",          no_argument,       0, 'h'},
        {"max-alive",     required_argument, 0, 'a'},
        {"max-total",     required_argument, 0, 't'},
        {"min-radius",    required_argument, 0, 'r'},
        {"padding",       required_argument, 0, 'p'},
        {"grow-by",       required_argument, 0, 'g'},
        {"edge-color",    required_argument, 0, 'c'},
        {"input-file",    required_argument, 0, 'i'},
        {"input-format",  required_argument, 0, 'f'},
        {"output-file",   required_argument, 0, 'o'},
        {"output-format", required_argument, 0, 'F'},
        {0,               0,                 0, 0}
    };
    opterr = 1; // have getopt show error messages for us

    while ((rc = getopt_long(argc, argv, options, long_options,
                    &option_index)) != -1) {
        switch (rc) {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            case 'a':
                max_alive = strtol(optarg, NULL, 10);
                break;
            case 't':
                max_total = strtol(optarg, NULL, 10);
                break;
            case 'r':
                min_radius = strtol(optarg, NULL, 10);
                break;
            case 'p':
                padding = strtol(optarg, NULL, 10);
                break;
            case 'g':
                grow_by = strtol(optarg, NULL, 10);
                break;
            case 'c':
                strncpy(edge_color_str, optarg, 7);
                break;
            case 'i':
                strncpy(input_filename, optarg, 255);
                break;
            case 'f':
                strncpy(input_format, optarg, 7);
                break;
            case 'o':
                strncpy(output_filename, optarg, 255);
                break;
            case 'F':
                strncpy(output_format, optarg, 7);
                break;
            case '?':
                // error message handled by getopt
                usage();
                exit(EXIT_FAILURE);
            default:
                fprintf(stderr, "Undefined option 0x%0x\n", rc);
                usage();
                exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        fprintf(stderr, "circlefit: Unexpected trailing argument '%s'\n", argv[optind]);
        usage();
        exit(EXIT_FAILURE);
    }

    if (max_alive < 1) {
        fprintf(stderr, "circlefit: max-alive must be at least 1\n");
        usage();
        exit(EXIT_FAILURE);
    }
    if (max_total < 0) {
        fprintf(stderr, "circlefit: max-total must be at least 0\n");
        usage();
        exit(EXIT_FAILURE);
    }
    if (min_radius < 1) {
        fprintf(stderr, "circlefit: min-radius must be at least 1\n");
        usage();
        exit(EXIT_FAILURE);
    }
    if (padding < 0) {
        fprintf(stderr, "circlefit: padding must be at least 0\n");
        usage();
        exit(EXIT_FAILURE);
    }
    if (grow_by < 1) {
        fprintf(stderr, "circlefit: grow-by must be at least 1\n");
        usage();
        exit(EXIT_FAILURE);
    }

    // TODO check and handle other options

    // TODO error checking
    size_t bmp_size;
    char *bmp_file;
    if (img_format == PNG) {
        if (use_stdin)
            read_png_stdio(&orig_png, &orig_png_buf);
        else
            read_png_file(&orig_png, &orig_png_buf, "test.png");
        img_width = orig_png.width;
        img_height = orig_png.height;
    } else if (img_format == BMP) {
        if (use_stdin)
            bmp_file = read_bmp_stdio(&bmp_size);
        else
            bmp_file = read_file("test.bmp", &bmp_size);
        decode_bmp(&orig_bmp, &bmp_callbacks, bmp_file, bmp_size);
        img_width = orig_bmp.width;
        img_height = orig_bmp.height;
    }

    srand(time(NULL));

    // Circle generation algorithm
    // Based on XScreenSaver boxfit by jwz

    // allocate initial boxes storage
    int boxes_size = 2 * max_alive;
    boxes = calloc(boxes_size, sizeof(*boxes));
    if (!boxes) {
        fprintf(stderr, "Failed to allocate memory for %d boxes\n", boxes_size);
        exit(EXIT_FAILURE);
    }

    // start circle placement
    nboxes = 0; // total number of existing boxes
    int nalive = 0; // number of living boxes
    bool finished = false;

    while (!finished) {
        // grow boxes if possible
        for (int i = 0; i < nboxes; i++) {
            box *b = &boxes[i];

            if (!b->alive) {
                // don't keep growing, it's already dead
            } else if (!box_legal(b, grow_by + padding)) {
                // can't grow anymore, make it dead
                b->alive = false;
                nalive--;
            } else {
                // grow the box
                b->r += grow_by;
            }
        }

        // add new boxes if needed
        while (nalive < max_alive) {
            if (boxes_size <= nboxes) {
                // need to reallocate
                boxes_size = (1.5 * boxes_size) + nboxes;
                boxes = realloc(boxes, boxes_size * sizeof(*boxes));
                if (!boxes) {
                    fprintf(stderr, "Failed to allocate memory for %d boxes\n",
                            boxes_size);
                    exit(EXIT_FAILURE);
                }
            }

            // try to add a new box 100 times
            box *b = &boxes[nboxes];
            b->alive = false;
            for (int i = 0; i < 100; i++) {
                b->x = padding + (rand() % (img_width - 2*padding));
                b->y = padding + (rand() % (img_height - 2*padding));
                b->r = min_radius;

                if (box_legal(b, padding)) {
                    // successfully found a spot
                    b->alive = true;
                    nboxes++;
                    nalive++;
                    break;
                }
            }
            if (!b->alive || nboxes >= max_total) {
                // unable to find a new box to add, or reached max
                finished = true;
                break;
            }
        }
    }

    outbuf = calloc(img_width * img_height, sizeof(pixel));
    if (!outbuf) {
        fprintf(stderr, "Failed to allocate %zu bytes\n",
                img_width * img_height * sizeof(pixel));
        exit(EXIT_FAILURE);
    }

    // draw boxes
    for (int i = 0; i < nboxes; i++) {
        box *b = &boxes[i];
        draw_box(b, getpixel(b->x, b->y), edge_color);
    }

    // TODO add libpng output to file or stdio, with error checking
    fwrite(outbuf, sizeof(pixel), img_width * img_height, stdout);

    if (img_format == PNG) {
        png_image_free(&orig_png);
        free(orig_png_buf);
    } else if (img_format == BMP) {
        bmp_finalise(&orig_bmp);
        free(bmp_file);
    }

    free(boxes);
    free(outbuf);

    return 0;
}


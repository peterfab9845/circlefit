# circlefit
A version of `boxfit` from jwz's XScreenSaver that produces a static image instead of animating.
Intended to generate partially-obscured screenshots for use as lockscreen images.

## Features
* Various tunable circle algorithm parameters
  * Parameters result in different amounts of obscurity
* PNG or BMP input from file or stdin
* Raw 24-bit RGB output to file or stdout (PNG support planned)

## Sample
The following image shows a screenshot obscured with `circlefit`.

![Sample screenshot](https://i.imgur.com/OKxVMJZ.png)

## Usage
PNG output is not yet implemented. Currently, only raw 24bpp RGB output is available (to file or to stdout).

Please note that there can be noticeable speed differences based on the image formats used.
BMP input and raw output will likely be the fastest modes.

An example using [`maim`](https://github.com/naelstrof/maim) and [`i3lock`](https://github.com/i3/i3lock):
```
maim -f bmp | circlefit | i3lock --raw 1920x1080:rgb --image /dev/stdin
```

## Requirements
* [libpng](http://www.libpng.org/pub/png/libpng.html) (PNG support)
* [libnsbmp](https://www.netsurf-browser.org/projects/libnsbmp/) (BMP support)

## Credits
* Inspiration and circle placement algorithm taken from [XScreenSaver](https://www.jwz.org/xscreensaver/) `boxfit` by Jamie Zawinski
* Bresenham's Circle Algorithm based on code from a [blog post](https://funloop.org/post/2021-03-15-bresenham-circle-drawing-algorithm.html) by Linus Arver

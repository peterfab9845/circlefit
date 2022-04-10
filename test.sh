#!/bin/sh

NAME="${NAME:-circlefit}"
testno=1

# INPUT FORMATS

echo "Test ${testno}: BMP from stdin, bmp format default"
maim -u -f bmp | ./${NAME} | convert -size 3840x1130 -depth 8 RGB:- out${testno}.png
testno=$((testno+1))

echo "Test ${testno}: BMP from stdin, bmp format specified"
maim -u -f bmp | ./${NAME} -f bmp | convert -size 3840x1130 -depth 8 RGB:- out${testno}.png
testno=$((testno+1))

echo "Test ${testno}: BMP from .bmp file, bmp format guessed/default"
maim -u input.bmp; ./${NAME} -i input.bmp | convert -size 3840x1130 -depth 8 RGB:- out${testno}.png
testno=$((testno+1))

echo "Test ${testno}: BMP from .bmp file, bmp format specified"
maim -u input.bmp; ./${NAME} -i input.bmp -f bmp | convert -size 3840x1130 -depth 8 RGB:- out${testno}.png
testno=$((testno+1))

echo "Test ${testno}: BMP from .asd file, bmp format specified"
maim -u -f bmp input.asd; ./${NAME} -i input.asd -f bmp | convert -size 3840x1130 -depth 8 RGB:- out${testno}.png
testno=$((testno+1))

echo "Test ${testno}: PNG from stdin, png format specified"
maim -u -f png | ./${NAME} -f png | convert -size 3840x1130 -depth 8 RGB:- out${testno}.png
testno=$((testno+1))

echo "Test ${testno}: PNG from .png file, png format guessed"
maim -u input.png; ./${NAME} -i input.png | convert -size 3840x1130 -depth 8 RGB:- out${testno}.png
testno=$((testno+1))

echo "Test ${testno}: PNG from .asd file, png format specified"
maim -u -f png input.asd; ./${NAME} -i input.asd -f png | convert -size 3840x1130 -depth 8 RGB:- out${testno}.png
testno=$((testno+1))

# OUTPUT FORMATS

# TODO


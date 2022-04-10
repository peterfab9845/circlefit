#!/bin/sh

NAME="${NAME:-circlefit}"
RESOLUTION="3840x1130"

testno=1
echo "INPUT FORMATS"

echo "Test ${testno}: BMP from stdin, bmp format default"
maim -u -f bmp | ./${NAME} | convert -size ${RESOLUTION} -depth 8 RGB:- out${testno}.png
testno=$((testno+1))

echo "Test ${testno}: BMP from stdin, bmp format specified"
maim -u -f bmp | ./${NAME} -f bmp | convert -size ${RESOLUTION} -depth 8 RGB:- out${testno}.png
testno=$((testno+1))

echo "Test ${testno}: BMP from .bmp file, bmp format guessed/default"
maim -u input.bmp; ./${NAME} -i input.bmp | convert -size ${RESOLUTION} -depth 8 RGB:- out${testno}.png
testno=$((testno+1))

echo "Test ${testno}: BMP from .bmp file, bmp format specified"
maim -u input.bmp; ./${NAME} -i input.bmp -f bmp | convert -size ${RESOLUTION} -depth 8 RGB:- out${testno}.png
testno=$((testno+1))

echo "Test ${testno}: BMP from .asd file, bmp format specified"
maim -u -f bmp input.asd; ./${NAME} -i input.asd -f bmp | convert -size ${RESOLUTION} -depth 8 RGB:- out${testno}.png
testno=$((testno+1))

echo "Test ${testno}: PNG from stdin, png format specified"
maim -u -f png | ./${NAME} -f png | convert -size ${RESOLUTION} -depth 8 RGB:- out${testno}.png
testno=$((testno+1))

echo "Test ${testno}: PNG from .png file, png format guessed"
maim -u input.png; ./${NAME} -i input.png | convert -size ${RESOLUTION} -depth 8 RGB:- out${testno}.png
testno=$((testno+1))

echo "Test ${testno}: PNG from .asd file, png format specified"
maim -u -f png input.asd; ./${NAME} -i input.asd -f png | convert -size ${RESOLUTION} -depth 8 RGB:- out${testno}.png
testno=$((testno+1))

echo
echo "OUTPUT FORMATS"

echo "Test ${testno}: raw to stdout, raw format default"
maim -u -f bmp | ./${NAME} > out${testno}.raw; convert -size ${RESOLUTION} -depth 8 RGB:out${testno}.raw out${testno}.png
testno=$((testno+1))

echo "Test ${testno}: raw to stdout, raw format specified"
maim -u -f bmp | ./${NAME} -F raw > out${testno}.raw; convert -size ${RESOLUTION} -depth 8 RGB:out${testno}.raw out${testno}.png
testno=$((testno+1))

echo "Test ${testno}: raw to .raw file, raw format guessed/default"
maim -u -f bmp | ./${NAME} -o out${testno}.raw; convert -size ${RESOLUTION} -depth 8 RGB:out${testno}.raw out${testno}.png
testno=$((testno+1))

echo "Test ${testno}: raw to .raw file, raw format specified"
maim -u -f bmp | ./${NAME} -o out${testno}.raw -F raw; convert -size ${RESOLUTION} -depth 8 RGB:out${testno}.raw out${testno}.png
testno=$((testno+1))

echo "Test ${testno}: raw to .asd file, raw format default"
maim -u -f bmp | ./${NAME} -o out${testno}.asd; convert -size ${RESOLUTION} -depth 8 RGB:out${testno}.asd out${testno}.png
testno=$((testno+1))

echo "Test ${testno}: raw to .asd file, raw format specified"
maim -u -f bmp | ./${NAME} -o out${testno}.asd -F raw; convert -size ${RESOLUTION} -depth 8 RGB:out${testno}.asd out${testno}.png
testno=$((testno+1))

echo "Test ${testno}: PNG to stdout, png format specified"
maim -u -f bmp | ./${NAME} -F png > out${testno}.png
testno=$((testno+1))

echo "Test ${testno}: PNG to .png file, png format guessed"
maim -u -f bmp | ./${NAME} -o out${testno}.png -F png
testno=$((testno+1))

echo "Test ${testno}: PNG to .asd file, png format specified"
maim -u -f bmp | ./${NAME} -o out${testno}.asd -F png
testno=$((testno+1))


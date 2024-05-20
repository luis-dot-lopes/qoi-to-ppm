# QOI to PPM image format converter

This is a simple C program to convert from the [QOI image format](https://qoiformat.org/) to the [PPM image format](https://en.wikipedia.org/wiki/Netpbm).

To test it, you can download images in the QOI format at their website.

To compile:

```
gcc main.c
```

Usage:

```
./a.out [filename]
```

The output is written to out.ppm

You can also convert PPM images to QOI:

```
./a.out [filename] -qoi
```

The output is written to out.qoi

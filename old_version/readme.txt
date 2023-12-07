Old Version:
The programs require GNU gcc compiler

1. On the prompt of Windows 11, run t.bat.

2. type test "image.jpg" on the prompt.

The output bitmap file: bitmapImage.bmp

We finished and solved the following problems:
1. entropy decode
2. dequantization
3. correctly generate RGB values of 8x8 blocks
4. bitstream analyzer jpeg, use gcc -o bsa bitstreamAnalyzer.c

We try our best and spend much time to write and debug the JPEG decoder.
Eventually, we finish and test the complete JPEG program in the new version.

https://github.com/lukaresearch/JPEG_HW

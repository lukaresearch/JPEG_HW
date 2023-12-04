# JPEG decoder
 
The programs require GNU gcc compiler

1. On the prompt of Windows OS 11, run t.bat.

2. type test "image.jpg" on the prompt.

The output bitmap file: bitmapImage.bmp

We finished and solved the following problems:
1. entropy decode
2. dequantization
3. correctly generate RGB values of 8x8 blocks
4. bitstream analyzer jpeg, use gcc -o bsa bitstreamAnalyzer.c

We try our best and spend much time on debugging to write the JPEG decoder, but the program still cannot output the correct bitmap image.

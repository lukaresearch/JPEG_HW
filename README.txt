A Simple and Fast JPEG Decoder C program

1. fastDecoder.c (main routines)
	readMarker()
		defineHtable() (entropy encoding Huffman and run-length tables)
	scanMCU()
		blockEntropyDecode()
		blockDequantize()
		iDCT()
		YCbCr2RGB()
	BitmapWriter() (write RGB to bitmap file)

2. Header File jpeg.h

3. simple bitstreamAnalyzer.c (gcc -o bitstream bitstreamAnalyzer.c)

To compile in Windows cmd environment (command prompt) with GNU C compiler gcc:
	gcc -o decoder fastDecoder.c

To run:
	decoder monalisa.jpg monalisa.bmp
	decoder gig-sn01.jpg gig-sn01.bmp
	decoder gig-sn08.jpg gig-sn08.bmp
	decoder teatime.jpg teatime.bmp

just simple and fast

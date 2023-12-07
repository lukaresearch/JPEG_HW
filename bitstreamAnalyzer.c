#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MarkerCode                      0xFF
#define StartOfImage                    0xD8
#define uchar                           unsigned char

#define Black                           "\033[1;30m"
#define Red                             "\033[1;31m"
#define Green                           "\033[1;32m"
#define Yellow                          "\033[1;33m"
#define Blue                            "\033[1;34m"
#define Purple                          "\033[1;35m"
#define Cyan                            "\033[1;36m"
#define White                           "\033[1;37m"

#define lineLen                         32  // line width of hex bitstream map

void resetColor() { printf("\033[0m"); }

void printChar( int p, uchar c, char *color ) {
    if( (p % lineLen) == 0 ) {
        printf(White);
        printf("\n%06X:", p);
    }
    printf(color);
    printf(" %02X", c);
}

void printMarker(int p, uchar char1, uchar char2, char *color1, char *color2) {
    printChar( p-2, char1, color1 );
    printChar( p-1, char2, color2 );
}

void printChunk(int p, int lenChunk, char *jpgData) {
    for( int i = 0; i<lenChunk; i++ )
        printChar( p+i, *(jpgData+p+i), White );
}

int main(int argc, char** argv) {
    char * jpegFileName;
    int lenChunk, endPoint = 0x2FF;

    if( argc < 2 ) {
        jpegFileName = "teatime.jpg";
    }
    else
        jpegFileName = argv[1];

// Files
    FILE * file = fopen(jpegFileName, "r+");
    if (file == NULL) return 1;
    fseek(file, 0, SEEK_END);
    long int fileSize = ftell(file);
    fclose(file);
    printf("input file size = %d bytes\n", fileSize);

    file = fopen(jpegFileName, "rb");
    uchar * jpgData = (uchar *) malloc(fileSize);
    int bytes_read = fread(jpgData, sizeof(uchar), fileSize, file);
    fclose(file);
    printf("bytes read = %d bytes\n", bytes_read);

///////////////////////////////////////////////////////////////
// header part of JPEG bitstream
    int p = 0;  // position
    while( p < fileSize ) {
        uchar c = *(jpgData + p);
        printChar( p, c, White );
        p++;
        if( p>endPoint ) break;
    }

    printf("\n\n");
    p = 0;  // position
    while( p < fileSize ) {
        uchar markChar = *(jpgData + p);      
        if( markChar != MarkerCode ) break;   // end of header
        p++;

        markChar = *(jpgData + p);
        p++;
 
        if( markChar == StartOfImage ) {
            printMarker(p, 0xFF, markChar, Red, Red );    // identify markers 0xFFD8
        }
        else {
            printMarker(p, 0xFF, markChar, Red, Yellow );
            lenChunk = ((((short) *(jpgData+p)) << 8) | (0x00FF & (*(jpgData+p+1))));
            printChunk( p, lenChunk, jpgData );
            p += lenChunk;
        }
        resetColor();
    }
    //printf("\np=%02X\n", p);

/////////////////////////////////////////////////
// start scanning entropy-encoded JPEG bitstream
    printChar( p, *(jpgData + p), Blue );
    p++;
    while( p < fileSize ) {
        uchar c = *(jpgData + p);
        if( c == 0xFF )
            printChar( p, c, Red ); // must be followed by redundant 0x00
        else
            printChar( p, c, Blue );

        p++;

        if( p>endPoint ) break;
    }

    resetColor();
    free(jpgData);
    return 0;
}
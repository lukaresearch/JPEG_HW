#include "jpeg.h"

const uchar ZigZag[] = {
    0,   1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63 };

void resetColor() { printf("\033[0m"); }

void printChar( int q, uchar c, char *color ) {
    if( (q % lineLen) == 0 ) {
        resetColor();
        //printf("\n%06x:", q);
    }

    //printf(color);
    //printf(" %02X", c);
}

void printWord(uchar char1, uchar char2, char *color1, char *color2) {
    printChar( p-2, char1, color1 );
    printChar( p-1, char2, color2 );
}

void printChunk(int len) {
    for( int i = 0; i<len; i++ )
        printChar( p+i, *(jpgData+p+i), White );
}

uint readWord(int p) {
    return ((((short) *(jpgData+p)) << 8) | (0x00FF & (*(jpgData+p+1))));
}

void defineHtable(struct Htable * h) {
    int i, j, Nsym;
    uint code;

    h->offset[0] = 0;
    Nsym = 0;
    for (i=1; i <17; i++) {
        Nsym += *(jpgData+p+2+i);
        h->offset[i] = Nsym;
    }

    for (i=0; i < Nsym; i++) 
        h->symbol[i] = *(jpgData+p+19+i);

    code = 0;
    for (i=0; i<16; i++) {
        for ( j=h->offset[i]; j<h->offset[i+1]; j++) {
            h->code[j] = code;
            code += 1;
        }
        code <<= 1;
    }
/*
    printf("<=len offset\n");
    for (i=0; i <17; i++)
        printf("[%2d] %4d\n", i, h->offset[i]);

    printf("\n      symb code\n", Nsym);
    for (i=0; i <Nsym; i++)
        printf("[%3d] %04X %04X\n", i, h->symbol[i], h->code[i]);
*/
}

void readMarker() {
    uchar tableID;
    int i, j, pc;
    bool startScan = false;

    p = 0;  // position
    while( (p < fileSize) && (!startScan) ) {
        uchar markChar = *(jpgData + p);      
        p++;

        if( markChar != MarkerCode ) {
            printf("JPEG syntax error\n");
            break;
        }

        markChar = *(jpgData + p);
        p++;

        if( markChar == StartOfImage ) {
            printWord(0xFF, markChar, Red, Red );
        }
        else if( markChar == EndOfImage ) {
            printWord(0xFF, markChar, Red, Red );
            break;             
        }
        else {
            printWord(0xFF, markChar, Red, Yellow );
            int lenChunk = readWord(p);
            //printChunk(lenChunk);
        
            switch (markChar) {
                case SOFbaselineDCT:            // 0xC0 start of FRAME
                    height = readWord(p+3);
                    width = readWord(p+5);
                    printf("Start of Frame (baseline DCT)\nImageSize = %dx%d\n", height, width);
                    Ncomponent = *(jpgData+p+7); // always = 3 (0:Y, 1:Cb, 2:Cr)
                    for( i=0; i<Ncomponent; i++ ) { // i=0 means component 1 (Y, luminance)
                        pc = p + 8 + 3*i;
                        uint samplingFactor = *(jpgData+pc+1);
                        YCbCr[i].horizontalSamplingFactor = samplingFactor >> 4;
                        YCbCr[i].verticalSamplingFactor = samplingFactor & 0x4F;
                        YCbCr[i].QtableID = *(jpgData+pc+2);
                        printf("Component[%d] sampling Factor: %d %d\n",
                            i, YCbCr[i].horizontalSamplingFactor, YCbCr[i].verticalSamplingFactor);
                    }

                    blockHeight = (height + 7) / 8;
                    blockWidth = (width + 7) / 8;
                    blockHeightReal = blockHeight + (blockHeight % 2);
                    blockWidthReal = blockWidth + (blockWidth % 2);

                    blocks = malloc(blockHeightReal * blockWidthReal * sizeof(*blocks));        
                    
                    horizontalSamplingFactor = YCbCr[0].horizontalSamplingFactor;
                    verticalSamplingFactor = YCbCr[0].verticalSamplingFactor;
                    break;

                case DefineQuantizationTable:   // 0xDB
                    tableID = *(jpgData+p+2);
                    if( (tableID >=0) && (tableID<4) ) {
                        for( i=0; i<64; i++ )
                                qtable[tableID].table[ZigZag[i]] = *(jpgData+p+3+i);
                        printf("define Quantization Table[%d]\n", tableID);
                    }
                    break;
                    
                case DefineHuffmanTable:        // 0xC4
                    tableID = *(jpgData+p+2);
                    printf("define Huffman Table[%d,%d]\n", tableID & 0x0F, tableID >> 4);
                    defineHtable( &htable[tableID & 0x0F][tableID >> 4] ); // [][0]: DC,  [][1]: AC
                    break;

                case StartOfScan:               // 0xDA
                    printf("Start of Scan\n");
                    Ncomponent = *(jpgData+p+2);
                    if( Ncomponent != 3 )
                        printf("Warning: Ncomponent != 3\n");

                    for( i=0; i<Ncomponent; i++ ) {
                        pc = p + 3 + 2*i;
                        j = *(jpgData+pc) - 1;  // component 1,2,3 -> 0,1,2
                        if( j<0 ) { printf("Warning: component ID < 0\n"); return; }
                        uchar uc = *(jpgData+pc+1);
                        YCbCr[j].HtableID_DC = uc >> 4;
                        YCbCr[j].HtableID_AC = uc & 0x4F;
                        printf("   Htable: DC=%d, AC=%d\n", YCbCr[j].HtableID_DC, YCbCr[j].HtableID_AC);
                    }
                    startScan = true;
                    break;

                default:
                    break;
            }
            p += lenChunk;
        }
    }
    if( !startScan )
        printf("Error: not start scan\n");
}

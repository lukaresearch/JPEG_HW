#include "jpeg.h"
uchar tableID, samplingFactor, uc;
int i, j, lenChunk, pc;
bool startScan;
uchar bitPosition=0, dataByte=0;
int previousDC[3] = {0};
int blockCnt = 0, rgbCnt = 0;
int qtIndex[3] = {0, 1, 1};

uint getBit() {
    uchar bit;
    if( bitPosition == 0 ) {
        dataByte = *(jpgData + p);
        p++;
        if( dataByte == 0xFF ) {
            uchar v = *(jpgData + p);
            p++;
            if( v != 0x00 ) // 0xFF00 means 0xFF
                printf("Error: 0xFF not followed by 0x00\n");
        }
    }

    bit = (dataByte >> (7-bitPosition)) & 1;
//    printf(" bitPosition=%d, dataByte=%2X\n", bitPosition, dataByte);
    bitPosition = (bitPosition + 1) % 8;
    return bit;
}

uint getBits(uint len) {
    uint bit, bits, i;
    bits = 0;
    for( i=0; i<len; i++) {
        bit = getBit();
        bits = (bits << 1) | bit;
    }
    return bits;
}

uchar nextSym(struct Htable *h) {
    uint code, i, j;
    int bit;

/*
    printf("\n using Htable\n");
    for (i=0; i <17; i++)
        printf("[%2d]%4d\n", i, h->offset[i]);
*/
    code = 0;
    for( i=0; i<16; i++ ) {
        bit = getBit();
        code = (code << 1) | bit;
        for( j=h->offset[i]; j<h->offset[i+1]; j++)
            if( code == h->code[j] ) {
//                printf("[%2d] (code, symbol) = (%X, %X)\n", i, code, h->symbol[j]); // debug
                return h->symbol[j];
            }
    }

    return -1;
}

void printBlockIndex(uint componentIndex, uint blockIndex) {
    printf("Block %d\n", blockCnt);
    for (int i=0; i<8; i++) {
        for (int j=0; j<8; j++)
            printf("%4d", blocks[blockIndex].YCbCr[componentIndex][8*i+j]);
        printf("\n");
    }
}

void blockEntropyDecode(uint componentIndex, uint blockIndex) {
    uint i, j, m, len;
    // DC
    //printf("blockEntropyDecode\n");
    len = nextSym(&htable[YCbCr[componentIndex].HtableID_DC][0]);
    int c = getBits(len);
    if( (len > 0) && (c < (1 << (len-1))) )
        c = c - ((1 << len) - 1);
//    printf("DC: len=%d, pow=%d, pow1=%d, c=%d\n", len, (1 << (len-1)), ((1 << len) - 1), c);
    blocks[blockIndex].YCbCr[componentIndex][0] = c + previousDC[componentIndex];
    previousDC[componentIndex] = blocks[blockIndex].YCbCr[componentIndex][0];

    // AC
    //printf("\nScanMCU AC:\n");
    for( m=1; m<64; m++ )
        blocks[blockIndex].YCbCr[componentIndex][m] = 0;

    int nAC = 0;
    for( m=1; m<64; m++ ) {
        nAC++;
        //printf("\nAC[%d] m=%d\n", nAC, m);
        uchar s = nextSym(&htable[YCbCr[componentIndex].HtableID_AC][1]);
        if (s == 0x00) break;  // no more nonzero AC values

        uchar zeroRunLength = s >> 4;
        m += zeroRunLength; // skip zeros

        if( m>63 ) {
            printf("\nError in AC runlength of zeros\n");
            return;
        }

        len = s & 0x0F;
        c = getBits(len);
        if( (len > 0) && (c < (1 << (len-1))) )
            c -= ((1 << len) - 1);
        blocks[blockIndex].YCbCr[componentIndex][ZigZag[m]] = c;
//        printf(" symbol=%02X, RL=%X, len=%X, zz=%d, c=%d = %d\n",
//            s, zeroRunLength, len, ZigZag[m], c, blocks[blockIndex].YCbCr[componentIndex][ZigZag[m]] );

        //return; // debug
    }

    //printBlockIndex(componentIndex, blockIndex);
}

void blockDequantize(uint componentIndex, uint blockIndex) {
    //printf("blockDequantize\n");
    int qIndex = qtIndex[componentIndex];
    for( int i=0; i<64; i++ ) 
        blocks[blockIndex].YCbCr[componentIndex][i] *= qtable[qIndex].table[i];
    //printBlockIndex(componentIndex, blockIndex);
}

void printRGB(uint blockIndex, uint cblockIndex) {
    //FILE* demor;
    //demor = fopen("demo_r.txt", "a+");
    printf("Block %d, rgbCnt=%d\n", blockCnt, rgbCnt);
    printf("R\n");
    for (int i=0; i<8; i++) {
        for (int j=0; j<8; j++){
            printf("%4d", blocks[blockIndex].R[8*i+j]);
            //fprintf(demor, "%4d", blocks[blockIndex].R[8*i+j]);
        }
        printf("\n");
        //fprintf(demor, "\n");
    }
    //fclose(demor);

    //FILE* demog;
    //demog = fopen("demo_g.txt", "a+");
    printf("G\n");
    for (int i=0; i<8; i++) {
        for (int j=0; j<8; j++){
            printf("%4d", blocks[blockIndex].G[8*i+j]);
            //fprintf(demog, "%4d", blocks[blockIndex].G[8*i+j]);
        }
        printf("\n");
        //fprintf(demog, "\n");
    }
    //fclose(demog);

    //FILE* demob;
    //demob = fopen("demo_b.txt", "a+");
    printf("B\n");
    for (int i=0; i<8; i++) {
        for (int j=0; j<8; j++){
            printf("%4d", blocks[blockIndex].G[8*i+j]);
            //fprintf(demob, "%4d", blocks[blockIndex].B[8*i+j]);
        }
        printf("\n");
        //fprintf(demob, "\n");
    }
    //fclose(demob);   
}

void YCbCr2RGB(int blockIndex, int cblockIndex, uint vs, uint hs, uint v, uint h) {
    for (uint y = 7; y < 8; --y)
        for (uint x = 7; x < 8; --x) {
            uint pixel = y * 8 + x;
            const uint cpixel = (y / vs + 4 * v) * 8 + x / hs + 4 * h;
            int Y = blocks[blockIndex].YCbCr[0][pixel];
            int Cb = blocks[cblockIndex].YCbCr[1][cpixel];
            int Cr = blocks[cblockIndex].YCbCr[2][cpixel];
            int r = Y + 1.402f * Cr + 128;
            int g = Y - 0.344f * Cb - 0.714f * Cr + 128;
            int b = Y + 1.772f * Cb + 128;
            if (r < 0)   r = 0;
            if (r > 255) r = 255;
            if (g < 0)   g = 0;
            if (g > 255) g = 255;
            if (b < 0)   b = 0;
            if (b > 255) b = 255;
            blocks[blockIndex].R[pixel] = r;
            blocks[blockIndex].G[pixel] = g;
            blocks[blockIndex].B[pixel] = b;
        }
    printRGB(blockIndex, cblockIndex);
}

void scanMCU() {
    uint i, j, x, y, v, h, len;
    int c;
    printf("Scan MCU:\n   entropy decode\n   dequantize\n   iDCT\n   YCbCr2RGB\n");
    for( y=0; y< blockWidth; y += verticalSamplingFactor ) {
        for( x=0; x< blockHeight; x += horizontalSamplingFactor ) {
            for( i=0; i<Ncomponent; i++ ) {
                // printf("\ncomponent: %d\n", i);
                //printDCtable(i);
                //printACtable(i);

                for( v=0; v<YCbCr[i].verticalSamplingFactor; v++) {
                    for( h=0; h<YCbCr[i].horizontalSamplingFactor; h++) {
                        blockCnt++;
                        uint blockIndex =  (y+v)*blockWidthReal + x + h;
                        blockEntropyDecode(i, blockIndex);
                        blockDequantize(i, blockIndex);
                        iDCT(&blocks[blockIndex].YCbCr[i][0]);

//                        if(blockCnt>3) return;
//                        return;  // debug
                    }
                }
            }

            uint cblockIndex = y * blockWidthReal + x;
            for( v=verticalSamplingFactor-1; v<verticalSamplingFactor; v--) {
                for( h=horizontalSamplingFactor-1; h<horizontalSamplingFactor; h--) {
                    uint blockIndex =  (y+v)*blockWidthReal + x + h;
                    rgbCnt++;
//                    printf("(x,y,i,v,h)=(%d,%d,%d,%d,%d)\n", x,y,i,v,h);
                    YCbCr2RGB(blockIndex, cblockIndex,
                        verticalSamplingFactor, horizontalSamplingFactor, v, h);
//                        if(blockCnt>3) return;
                    //return;  // debug
                }
            }
            //return;   
        }
    }
    printf("%d 8x8 blocks processed\n", blockCnt);
    displayBlocks();
}

int main(int argc, char** argv) {
    if( argc < 2 ) {
        printf("Error!\n");
    }
    else if( argc == 2 ) {
        jpgFileName = argv[1];
        bmpFileName = "output.bmp";
    }
    else {
        jpgFileName = argv[1];
        bmpFileName = argv[2];
    }

    if( !fileIO() ) {
        printf("Unable to read file\n");
        return -1;
    }

    readMarker();   // marker header segment
    scanMCU();      // entropy-coded segment
    BitmapWriter();

    free(jpgData);
    free(blocks);
    return 0;
}
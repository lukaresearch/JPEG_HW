// a fast JPEG decoder in C language
#include "jpeg.h"
uchar tableID, samplingFactor, uc;
int i, j, lenChunk, pc;
bool startScan;
uchar bitPosition=0, dataByte=0;
int DC[3] = {0};    // previous DC values
int blockCnt = 0;
int qtIndex[3] = {0, 1, 1};

const uchar ZigZag[] = {
    0,   1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63 };

uint readWord(int p) {
    return ((((short) *(jpgData+p)) << 8) | (0x00FF & (*(jpgData+p+1))));
}

bool fileIO() {
    printf("Input JPG File: %s\nOutput File: %s\n", jpgFileName, bmpFileName);
    FILE *jpgFile = fopen(jpgFileName, "r+");
    if (jpgFile == NULL) {
        printf("Unable to open File %s\n", jpgFileName);
        return false;
    }
    
    fseek(jpgFile, 0, SEEK_END);
    fileSize = ftell(jpgFile);
    fclose(jpgFile);

    jpgFile = fopen(jpgFileName, "rb");
    jpgData = (uchar *) malloc(fileSize);
    int uchars_read = fread(jpgData, sizeof(uchar), fileSize, jpgFile);
    fclose(jpgFile);
    
    printf("input file size = %d bytes\n", fileSize);
    return true;
}

void BitmapWriter() {
    printf("Running bitMapWriter...\n");
  
    if(sizeof(struct BITMAPFILEHEADER) != 14 && sizeof(struct BITMAPINFOHEADER) != 40)
        printf("bitmap structures not packed properly\n");

    int imagesize = width * height;   // in pixels
    uint paddingSize = width % 4;
    uint sizeByte = imagesize * 3 + paddingSize * height;

    struct BITMAPFILEHEADER filehdr = { 0 };    // 14-byte
    struct BITMAPINFOHEADER infohdr = { 0 };    // 40-byte

    memcpy(&filehdr, "BM", 2);          // default
    filehdr.bfSize = 54 + sizeByte;     // file size in bytes
    filehdr.bfOffBits = 54;             // 54=14+40

    infohdr.biSize = 40;                //sizeof(infohdr)
    infohdr.biPlanes = 1;               //number of planes = 1
    infohdr.biWidth = width;
    infohdr.biHeight = height;
    infohdr.biBitCount = 24;            // 3-byte RGB = 24 bits
    infohdr.biSizeImage = sizeByte;

    uchar * buf = malloc(sizeByte);
    int q = 0;
    for(int row = height - 1; row >= 0; row--) {
        const uint blockRow = row / 8;
        const uint pixelRow = row % 8;
        uint pad = row * paddingSize;
        for(int column = 0; column < width; column++)
        {
            const uint blockColumn = column / 8;
            const uint pixelColumn = column % 8;
            const uint blockIndex = blockRow * blockWidthReal + blockColumn;
            const uint pixelIndex = pixelRow * 8 + pixelColumn;

            buf[q++] = blocks[blockIndex].B[pixelIndex];
            buf[q++] = blocks[blockIndex].G[pixelIndex];
            buf[q++] = blocks[blockIndex].R[pixelIndex];
        }
        q += paddingSize;
    }
    printf("bitmap header size = %d + %d\n", sizeof(filehdr), sizeof(infohdr));
    printf("Image size = %d (pixels) = %d (bytes)\n", imagesize, sizeByte);
    FILE *fout = fopen(bmpFileName , "wb");
    fwrite(&filehdr, sizeof(filehdr), 1, fout);
    fwrite(&infohdr, sizeof(infohdr), 1, fout);
    fwrite((uchar*)buf, sizeByte, 1, fout);
    fclose(fout);
    free(buf);
}

void defineHtable() {
    int i, j, Nsym;

    printf("  DefindHuffmanTable\n");
    int len = readWord(p)-2;
    int p1 = p;
    while( len > 0 ) {
        uchar info = *(jpgData+p1+2);
        uchar tableID = info & 0x0F;
        uchar adc = info >> 4;
        printf("    Huffman Table[%d,%d]\n", info, adc);
        htable[tableID][adc].offset[0] = 0;
        Nsym = 0;
        for (i=1; i <17; i++) {
            Nsym += *(jpgData+p1+2+i);
            htable[tableID][adc].offset[i] = Nsym;
        }

        for (i=0; i < Nsym; i++) 
            htable[tableID][adc].symbol[i] = *(jpgData+p1+19+i);

        uint code = 0;
        for (i=0; i<16; i++) {
            for ( j=htable[tableID][adc].offset[i]; j<htable[tableID][adc].offset[i+1]; j++) {
                htable[tableID][adc].code[j] = code;
                code += 1;
            }
            code <<= 1;
        }
        len -= 17 + Nsym;
        p1 += 17 + Nsym;
    }
}

void readMarker() {     // read Marker header
    uchar tableID;
    int i, j, pc, len;
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
//            printf("Start of Image\n");
        }
        else if( markChar == EndOfImage ) {
//            printf("End of Image\n");
            break;             
        }
        else {
            int lenChunk = readWord(p);
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
                    blocksSize = blockHeightReal * blockWidthReal;

                    blocks = malloc(blocksSize * sizeof(*blocks));
                    
                    horizontalSamplingFactor = YCbCr[0].horizontalSamplingFactor;
                    verticalSamplingFactor = YCbCr[0].verticalSamplingFactor;
                    break;

                case DefineQuantizationTable:   // 0xDB
                    len = readWord(p)-2;
                    int p1 = p;
                    while( len > 0 ) {
                        tableID = *(jpgData+p1+2) & 0x0F;
                        len--;
                        if( (tableID >> 4) != 0 ) {
                            for( i=0; i<64; i++ )
                                qtable[tableID].table[ZigZag[i]] = readWord(p1+3+i);
                            len -= 128;
                            p1 += 129;
                        }
                        else {
                            for( i=0; i<64; i++ )
                                    qtable[tableID].table[ZigZag[i]] = *(jpgData+p1+3+i);
                            len -= 64;
                            p1 += 65;
                        }
//                        printf("    Qtable[%d]\n", tableID);
                    }
                    if( len != 0 ) {
                        printf("Error: DQT\n");
                        return;
                    }
                    break;
                    
                case DefineHuffmanTable:        // 0xC4
                    defineHtable(); // [][0]: DC,  [][1]: AC
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

void printQuantizationTable(uint i) {
    printf("Quantization Table ID: %d", i);
    for (uint j = 0; j < 64; ++j) {
        if (j % 8 == 0) printf("\n");
        printf(" %2d", qtable[i].table[j]);
    }
    printf("\n");
}

void printDCtable(uint i) {
    printf("DC Table ID: %2d\n", i);

    printf("symbol\n");
    for (uint j = 0; j < 16; ++j) {
        printf(" %2d:", j + 1);
        for (uint k = htable[i][0].offset[j]; k < htable[i][0].offset[j + 1]; ++k) {
            printf(" %X", htable[i][0].symbol[k]);
        }
        printf("\n");
    }

    printf("code\n");
    for(uint j=0; j<16; j++) {
        printf(" %2d:", j+1);
        for ( uint k=htable[i][0].offset[j]; k<htable[i][0].offset[j+1]; k++) {
            printf(" %X", htable[i][0].code[k]);
        }
        printf("\n");
    }        
}

void printACtable(uint i) {
    printf("AC Table ID: %2d\n", i);
    for (uint j = 0; j < 16; ++j) {
        printf(" %2d:", j + 1);
        for (uint k = htable[i][1].offset[j]; k < htable[i][1].offset[j + 1]; ++k) {
            printf(" %X", htable[i][1].symbol[k]);
        }
        printf("\n");
    }
}

void displayTables() {
    for (uint i=0; i<2; i++) printQuantizationTable(i);
    for (uint i=0; i<2; i++) printDCtable(i);
    for (uint i=0; i<2; i++) printACtable(i);
}

void displayBlocks() {
    printf("blockHeight = %d\n", blockHeight);
    printf("blockWidth = %d\n", blockWidth);
    printf("verticalSamplingFactor = %d\n", verticalSamplingFactor);
    printf("horizontalSamplingFactor = %d\n", horizontalSamplingFactor);
    printf("Number of Components = %d\n", Ncomponent);
}

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
    uint code=0, i, j;
    int bit;

    for( i=0; i<16; i++ ) {
        bit = getBit();
        code = (code << 1) | bit;
        for( j=h->offset[i]; j<h->offset[i+1]; j++)
            if( code == h->code[j] )
                return h->symbol[j];
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

    blocks[blockIndex].YCbCr[componentIndex][0] = c + DC[componentIndex];
    DC[componentIndex] = blocks[blockIndex].YCbCr[componentIndex][0];

    // AC
    for( m=1; m<64; m++ )
        blocks[blockIndex].YCbCr[componentIndex][m] = 0;

    for( m=1; m<64; m++ ) {
        //printf("\nAC[%d] m=%d\n", nAC, m);    // debug
        uchar s = nextSym(&htable[YCbCr[componentIndex].HtableID_AC][1]);
        if (s == 0x00) break;  // no more nonzero AC values

        uchar zeroRunLength = s >> 4;
        m += zeroRunLength; // skip zeros

        len = s & 0x0F;
        c = getBits(len);
        if( (len > 0) && (c < (1 << (len-1))) )
            c -= ((1 << len) - 1);
        blocks[blockIndex].YCbCr[componentIndex][ZigZag[m]] = c;
    }
    //printBlockIndex(componentIndex, blockIndex);      // debug entropy-decoding results
}

void blockDequantize(uint componentIndex, uint blockIndex) {
    //printf("blockDequantize\n");
    int qIndex = qtIndex[componentIndex];
    for( int i=0; i<64; i++ ) 
        blocks[blockIndex].YCbCr[componentIndex][i] *= qtable[qIndex].table[i];
    //printBlockIndex(componentIndex, blockIndex);      // debug dequantization results
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
    //printRGB(blockIndex, cblockIndex);        // debug YCbCr to RGB results
}

void scanMCU() {    // scan entropy-encoded data
    uint i, j, x, y, v, h, len;
    int c;
    printf("Scan MCU:\n   entropy decode\n   dequantize\n   iDCT\n   YCbCr2RGB\n");
    for( y=0; y< blockHeight; y += verticalSamplingFactor ) {
        for( x=0; x< blockWidth; x += horizontalSamplingFactor ) {
            for( i=0; i<Ncomponent; i++ ) {
                for( v=0; v<YCbCr[i].verticalSamplingFactor; v++) {
                    for( h=0; h<YCbCr[i].horizontalSamplingFactor; h++) {
                        blockCnt++;
                        uint blockIndex =  (y+v)*blockWidthReal + x + h;

                        blockEntropyDecode(i, blockIndex);

                        blockDequantize(i, blockIndex);

                        iDCT(&blocks[blockIndex].YCbCr[i][0]);
                    }
                }
            }

            uint cblockIndex = y * blockWidthReal + x;
            for( v=verticalSamplingFactor-1; v<verticalSamplingFactor; v--) {
                for( h=horizontalSamplingFactor-1; h<horizontalSamplingFactor; h--) {
                    uint blockIndex =  (y+v)*blockWidthReal + x + h;
                    YCbCr2RGB(blockIndex, cblockIndex,
                        verticalSamplingFactor, horizontalSamplingFactor, v, h);
                }
            }
        }
    }
    printf("%d 8x8 blocks processed\n", blockCnt);
    displayBlocks();
}

void iDCT(int*  component) {
    // IDCT scaling factors
    float m0 = 2.0 * cos(1.0 / 16.0 * 2.0 * M_PI);
    float m1 = 2.0 * cos(2.0 / 16.0 * 2.0 * M_PI);
    float m3 = 2.0 * cos(2.0 / 16.0 * 2.0 * M_PI);
    float m5 = 2.0 * cos(3.0 / 16.0 * 2.0 * M_PI);
    float m2 = m0 - m5;
    float m4 = m0 + m5;

    float s0 = cos(0.0 / 16.0 * M_PI) / sqrt(8);
    float s1 = cos(1.0 / 16.0 * M_PI) / 2.0;
    float s2 = cos(2.0 / 16.0 * M_PI) / 2.0;
    float s3 = cos(3.0 / 16.0 * M_PI) / 2.0;
    float s4 = cos(4.0 / 16.0 * M_PI) / 2.0;
    float s5 = cos(5.0 / 16.0 * M_PI) / 2.0;
    float s6 = cos(6.0 / 16.0 * M_PI) / 2.0;
    float s7 = cos(7.0 / 16.0 * M_PI) / 2.0;
    
    float intermediate[64];

    for (uint i = 0; i < 8; ++i) {
         float g0 = component[0 * 8 + i] * s0;
         float g1 = component[4 * 8 + i] * s4;
         float g2 = component[2 * 8 + i] * s2;
         float g3 = component[6 * 8 + i] * s6;
         float g4 = component[5 * 8 + i] * s5;
         float g5 = component[1 * 8 + i] * s1;
         float g6 = component[7 * 8 + i] * s7;
         float g7 = component[3 * 8 + i] * s3;

         float f0 = g0;
         float f1 = g1;
         float f2 = g2;
         float f3 = g3;
         float f4 = g4 - g7;
         float f5 = g5 + g6;
         float f6 = g5 - g6;
         float f7 = g4 + g7;

         float e0 = f0;
         float e1 = f1;
         float e2 = f2 - f3;
         float e3 = f2 + f3;
         float e4 = f4;
         float e5 = f5 - f7;
         float e6 = f6;
         float e7 = f5 + f7;
         float e8 = f4 + f6;

         float d0 = e0;
         float d1 = e1;
         float d2 = e2 * m1;
         float d3 = e3;
         float d4 = e4 * m2;
         float d5 = e5 * m3;
         float d6 = e6 * m4;
         float d7 = e7;
         float d8 = e8 * m5;

         float c0 = d0 + d1;
         float c1 = d0 - d1;
         float c2 = d2 - d3;
         float c3 = d3;
         float c4 = d4 + d8;
         float c5 = d5 + d7;
         float c6 = d6 - d8;
         float c7 = d7;
         float c8 = c5 - c6;

         float b0 = c0 + c3;
         float b1 = c1 + c2;
         float b2 = c1 - c2;
         float b3 = c0 - c3;
         float b4 = c4 - c8;
         float b5 = c8;
         float b6 = c6 - c7;
         float b7 = c7;

        intermediate[0 * 8 + i] = b0 + b7;
        intermediate[1 * 8 + i] = b1 + b6;
        intermediate[2 * 8 + i] = b2 + b5;
        intermediate[3 * 8 + i] = b3 + b4;
        intermediate[4 * 8 + i] = b3 - b4;
        intermediate[5 * 8 + i] = b2 - b5;
        intermediate[6 * 8 + i] = b1 - b6;
        intermediate[7 * 8 + i] = b0 - b7;
    }
    for (uint i = 0; i < 8; ++i) {
         float g0 = intermediate[i * 8 + 0] * s0;
         float g1 = intermediate[i * 8 + 4] * s4;
         float g2 = intermediate[i * 8 + 2] * s2;
         float g3 = intermediate[i * 8 + 6] * s6;
         float g4 = intermediate[i * 8 + 5] * s5;
         float g5 = intermediate[i * 8 + 1] * s1;
         float g6 = intermediate[i * 8 + 7] * s7;
         float g7 = intermediate[i * 8 + 3] * s3;

         float f0 = g0;
         float f1 = g1;
         float f2 = g2;
         float f3 = g3;
         float f4 = g4 - g7;
         float f5 = g5 + g6;
         float f6 = g5 - g6;
         float f7 = g4 + g7;

         float e0 = f0;
         float e1 = f1;
         float e2 = f2 - f3;
         float e3 = f2 + f3;
         float e4 = f4;
         float e5 = f5 - f7;
         float e6 = f6;
         float e7 = f5 + f7;
         float e8 = f4 + f6;

         float d0 = e0;
         float d1 = e1;
         float d2 = e2 * m1;
         float d3 = e3;
         float d4 = e4 * m2;
         float d5 = e5 * m3;
         float d6 = e6 * m4;
         float d7 = e7;
         float d8 = e8 * m5;

         float c0 = d0 + d1;
         float c1 = d0 - d1;
         float c2 = d2 - d3;
         float c3 = d3;
         float c4 = d4 + d8;
         float c5 = d5 + d7;
         float c6 = d6 - d8;
         float c7 = d7;
         float c8 = c5 - c6;

         float b0 = c0 + c3;
         float b1 = c1 + c2;
         float b2 = c1 - c2;
         float b3 = c0 - c3;
         float b4 = c4 - c8;
         float b5 = c8;
         float b6 = c6 - c7;
         float b7 = c7;

        component[i * 8 + 0] = b0 + b7 + 0.5f;
        component[i * 8 + 1] = b1 + b6 + 0.5f;
        component[i * 8 + 2] = b2 + b5 + 0.5f;
        component[i * 8 + 3] = b3 + b4 + 0.5f;
        component[i * 8 + 4] = b3 - b4 + 0.5f;
        component[i * 8 + 5] = b2 - b5 + 0.5f;
        component[i * 8 + 6] = b1 - b6 + 0.5f;
        component[i * 8 + 7] = b0 - b7 + 0.5f;
    }
    //printBlock(component);        // debug inverse DCT results
}

int main(int argc, char** argv) {
    if( argc < 2 ) {
        jpgFileName = "teatime.jpg";
        bmpFileName = "teatime.bmp";
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
    displayTables();
    scanMCU();      // entropy-coded segment
    printf("end of JPEG decoding\n");

    BitmapWriter();
    printf("%s generated\n", bmpFileName);
    free(jpgData);
    free(blocks);
    return 0;
}
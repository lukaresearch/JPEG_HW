#ifndef JPEG
#define JPEG

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#define lineLen                         32
#define winSize                         32768

#define MarkerCode                      0xFF

// start of frames
#define SOFbaselineDCT                  0xC0
//#define SOFextendedSequentialDCT        0xC1
//#define SOFprogressiveDCT               0xC2
//#define SOFlosslessSequential           0xC3

// table...
#define DefineHuffmanTable              0xC4 
//#define defineArithmeticCoding          0xCC

// other markers
#define StartOfImage                    0xD8
#define EndOfImage                      0xD9
#define StartOfScan                     0xDA
#define DefineQuantizationTable         0xDB
#define DefineNumberLines               0xDC
#define defineRestartInterval           0xDD
//#define defineHierarchicalProgression   0xDE
//#define ExpandReferenceComponent        0xDF
#define ApplicationSegment              0xE0
#define Comment                         0xFE
#define Data0xFF                        0x00

#define Black                           "\033[1;30m"
#define Red                             "\033[1;31m"
#define Green                           "\033[1;32m"
#define Yellow                          "\033[1;33m"
#define Blue                            "\033[1;34m"
#define Purple                          "\033[1;35m"
#define Cyan                            "\033[1;36m"
#define White                           "\033[1;37m"

#define uchar unsigned char
#define uint unsigned int

const uchar ZigZag[64];

char *jpgFileName, *bmpFileName;

int p;
long int fileSize;
uchar * jpgData;
int blockCnt;

struct BlockYCbCr {
    int YCbCr[3][64];
    int R[64], G[64], B[64];
};

struct Qtable {
    uint table[64];
};

struct Htable {
    uchar offset[17];
    uchar symbol[176];
    uint code[176];
};

struct Components {     // Y, Cb, Cr
    uchar horizontalSamplingFactor;
    uchar verticalSamplingFactor;
    uchar QtableID;
    uchar HtableID_DC;
    uchar HtableID_AC;
};

struct Qtable qtable[2];
struct Htable htable[2][2];
struct Components YCbCr[3];

#pragma pack(push, 1)
struct BITMAPFILEHEADER {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};

struct BITMAPINFOHEADER {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    
    //uint32_t biCompression;
    //uint32_t biSizeImage;
    //int32_t  biXPelsPerMeter;
    //int32_t  biYPelsPerMeter;
    //uint32_t biClrUsed;
    //uint32_t biClrImportant;
};
#pragma pack(pop)

uint height;
uint width;
uchar Ncomponent;

uint blockHeight;
uint blockWidth;
uint blockHeightReal;
uint blockWidthReal;
struct BlockYCbCr  *blocks;

uint horizontalSamplingFactor;
uint verticalSamplingFactor;

void readMarker();

void resetColor();
void printWord(uchar char1, uchar char2, char *color1, char *color2);
void printChunk(int len);
void defineHtable(struct Htable * h);
uint readWord(int p);
uint getBit();
uint getBits(uint len);
void entropyDecode(uint componentIndex, uint blockIndex);

void printBlock(int * component);
void printQuantizationTable(uint i);
void printDCtable(uint i);
void printACtable(uint i);
void display();
void displayBlocks();
bool fileIO();

void iDCT(int* component);
void BitmapWriter();
#endif
#include "jpeg.h"

void printQuantizationTable(uint i) {
    printf("\nQuantization Table ID: %2d\n", i);
    for (uint j = 0; j < 64; ++j) {
        if (j % 8 == 0)
            printf("\n");
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

void display() {
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

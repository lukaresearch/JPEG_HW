#include "jpeg.h"

void printBlock(int * component) {
        printf("Block component %d\n", blockCnt);
        for (uint i = 0; i < 8; ++i) {
            for (uint j = 0; j < 8; ++j)
                printf("%4d", component[8*i+j]);
            printf("\n");
        }
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
    //printBlock(component);
}
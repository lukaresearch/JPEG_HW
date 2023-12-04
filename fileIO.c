#include "jpeg.h"

const int BYTES_PER_PIXEL = 3; /// red, green, & blue
const int FILE_HEADER_SIZE = 14;
const int INFO_HEADER_SIZE = 40; 

void generateBitmapImage(unsigned char* image, int height, int width, char* imageFileName);
unsigned char* createBitmapFileHeader(int height, int stride);
unsigned char* createBitmapInfoHeader(int height, int width);

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
    //printf("uchars read = %d bytes\n", uchars_read);

    return true;
}
/*
void BitmapWriter1() {
    printf("bitMapWriter\n");
    if(sizeof(struct BITMAPFILEHEADER) != 14 && sizeof(struct BITMAPINFOHEADER) != 40)
        printf("bitmap structures not packed properly\n");

    int imagesize = width * height;   // in pixels
    uint sizeByte = imagesize * 3;
 
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
    for(int row = height - 1; row >= 0; row--)
    {
        const uint blockRow = row / 8;
        const uint pixelRow = row % 8;
        for(int column = 0; column < width; column++)
        {
            const uint blockColumn = column / 8;
            const uint pixelColumn = column % 8;
            const uint blockIndex = blockRow * blockWidthReal + blockColumn;
            const uint pixelIndex = pixelRow * 8 + pixelColumn;
            int q = (row * width + column) * 3;
            buf[q] = 0xFF;
            buf[q+1] = 0;
            buf[q+2] = 0;
        }
    }
    //printf("%d\n", sizeof(filehdr));
    printf("bitmap header size = %d + %d\n", sizeof(filehdr), sizeof(infohdr));
    printf("Image size = %d (pixels) = %d (bytes)\n", imagesize, sizeByte);
    FILE *fout = fopen(bmpFileName , "wb");
    fwrite(&filehdr, sizeof(filehdr), 1, fout);
    fwrite(&infohdr, sizeof(infohdr), 1, fout);
    fwrite((uchar*)buf, imagesize, 3, fout);
    fclose(fout);
    free(buf);
}

//*/
void BitmapWriter(){

    unsigned char image[height][width][BYTES_PER_PIXEL];
    char* imageFileName = (char*) "bitmapImage.bmp";

    int r, c;
    for (r = 0; r < height; r++) {
        const uint blockRow = r / 8;
        const uint pixelRow = r % 8;        
        for (c = 0; c < width; c++) {
            const uint blockColumn = c / 8;
            const uint pixelColumn = c % 8;
            const uint blockIndex = blockRow * blockWidthReal + blockColumn;
            const uint pixelIndex = pixelRow * 8 + pixelColumn;            
            image[r][c][2] = (unsigned char) (blocks[blockIndex].R[pixelIndex]); //red
            image[r][c][1] = (unsigned char) (blocks[blockIndex].G[pixelIndex]); //green
            image[r][c][0] = (unsigned char) (blocks[blockIndex].B[pixelIndex]); //blue
        }
    }

    generateBitmapImage((unsigned char*) image, height, width, imageFileName);
    printf("bitmapImage.bmp generated.");
}

void generateBitmapImage (unsigned char* image, int height, int width, char* imageFileName)
{
    int widthInBytes = width * BYTES_PER_PIXEL;

    unsigned char padding[3] = {0, 0, 0};
    int paddingSize = (4 - (widthInBytes) % 4) % 4;

    int stride = (widthInBytes) + paddingSize;

    FILE* imageFile = fopen(imageFileName, "wb");

    unsigned char* fileHeader = createBitmapFileHeader(height, stride);
    fwrite(fileHeader, 1, FILE_HEADER_SIZE, imageFile);

    unsigned char* infoHeader = createBitmapInfoHeader(height, width);
    fwrite(infoHeader, 1, INFO_HEADER_SIZE, imageFile);

    int i;
    for (i = 0; i < height; i++) {
        fwrite(image + (i*widthInBytes), BYTES_PER_PIXEL, width, imageFile);
        fwrite(padding, 1, paddingSize, imageFile);
    }

    fclose(imageFile);
}

unsigned char* createBitmapFileHeader (int height, int stride)
{
    int fileSize = FILE_HEADER_SIZE + INFO_HEADER_SIZE + (stride * height);

    static unsigned char fileHeader[] = {
        0,0,     /// signature
        0,0,0,0, /// image file size in bytes
        0,0,0,0, /// reserved
        0,0,0,0, /// start of pixel array
    };

    fileHeader[ 0] = (unsigned char)('B');
    fileHeader[ 1] = (unsigned char)('M');
    fileHeader[ 2] = (unsigned char)(fileSize      );
    fileHeader[ 3] = (unsigned char)(fileSize >>  8);
    fileHeader[ 4] = (unsigned char)(fileSize >> 16);
    fileHeader[ 5] = (unsigned char)(fileSize >> 24);
    fileHeader[10] = (unsigned char)(FILE_HEADER_SIZE + INFO_HEADER_SIZE);

    return fileHeader;
}

unsigned char* createBitmapInfoHeader (int height, int width)
{
    static unsigned char infoHeader[] = {
        0,0,0,0, /// header size
        0,0,0,0, /// image width
        0,0,0,0, /// image height
        0,0,     /// number of color planes
        0,0,     /// bits per pixel
        0,0,0,0, /// compression
        0,0,0,0, /// image size
        0,0,0,0, /// horizontal resolution
        0,0,0,0, /// vertical resolution
        0,0,0,0, /// colors in color table
        0,0,0,0, /// important color count
    };

    infoHeader[ 0] = (unsigned char)(INFO_HEADER_SIZE);
    infoHeader[ 4] = (unsigned char)(width      );
    infoHeader[ 5] = (unsigned char)(width >>  8);
    infoHeader[ 6] = (unsigned char)(width >> 16);
    infoHeader[ 7] = (unsigned char)(width >> 24);
    infoHeader[ 8] = (unsigned char)(height      );
    infoHeader[ 9] = (unsigned char)(height >>  8);
    infoHeader[10] = (unsigned char)(height >> 16);
    infoHeader[11] = (unsigned char)(height >> 24);
    infoHeader[12] = (unsigned char)(1);
    infoHeader[14] = (unsigned char)(BYTES_PER_PIXEL*8);

    return infoHeader;
}
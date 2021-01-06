#include "BMP.h"

int main()
{
    BMP file1 ("/Users/antonglomadov/CLionProjects/readBmp/2_24.bmp");
    BMP file2 ("/Users/antonglomadov/CLionProjects/readBmp/1_24.bmp");
    Painter(file1,file2,200,200,"/Users/antonglomadov/CLionProjects/readBmp/res_24.bmp");
    BMP file3 ("/Users/antonglomadov/CLionProjects/readBmp/1_32.bmp");
    BMP file4 ("/Users/antonglomadov/CLionProjects/readBmp/2_32.bmp");
    Painter(file3,file1,0,0,"/Users/antonglomadov/CLionProjects/readBmp/res_32.bmp");
    return 0;
}

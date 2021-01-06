#include <fstream>
#include <vector>
#include <stdexcept>
#include <iostream>


#pragma pack(push, 1)
struct bmpFileHeader{
    uint16_t signature{ 0x4D42 };
    uint32_t fileSize{ 0 };
    uint16_t reserved1{ 0 };
    uint16_t reserved2{ 0 };
    uint32_t offsetData{ 0 };
};
struct bmpInfo{
    uint32_t size{ 0 };
    int32_t width{ 0 };
    int32_t height{ 0 };
    uint16_t planes{ 1 };
    uint16_t bitCount{ 0 };
    uint32_t compression{ 0 };
    uint32_t sizeImage{ 0 };
    int32_t xPixelsPerMeter{ 0 };
    int32_t yPixelsPerMeter{ 0 };
    uint32_t colorsUsed{ 0 };
    uint32_t colorsImportant{ 0 };
};
struct colorTable{
    uint32_t redMask{0x00ff0000};
    uint32_t greenMask{0x0000ff00};
    uint32_t blueMask{0x000000ff};
    uint32_t alphaMask{0xff000000};
    uint32_t colorSpaceType{0x73524742};
    uint32_t unused[16]{0};
};
#pragma pack(pop)

class BMP{
public:
    bmpFileHeader fileHeader;
    bmpInfo bmpInformation;
    colorTable bmpColorInfo;
    std::vector<uint8_t> data;
    BMP (const char *fileName){

        read(fileName);
    }
    void read(const char *fileName){

        std::ifstream readFile(fileName, std::ios_base::binary);
        if (readFile){
            readFile.read(reinterpret_cast<char*>(&fileHeader),sizeof(fileHeader));
            if(fileHeader.signature != 0x4D42){
                throw std::runtime_error("ERROR! --- This isn't BMP file");
            }
            readFile.read(reinterpret_cast<char*>(&bmpInformation),sizeof(bmpInformation));
            if (bmpInformation.bitCount==32){
                if (bmpInformation.size >= sizeof(bmpInfo)+ sizeof(colorTable)){
                    readFile.read(reinterpret_cast<char*>(&bmpColorInfo),sizeof(bmpColorInfo));
                    checkColor(bmpColorInfo);
                }
                else{
                    throw std::runtime_error("ERROR! --- This isn't BMP file because doesn't content bit mask");
                }
            }
            readFile.seekg(fileHeader.offsetData,readFile.beg);

            if (bmpInformation.bitCount == 32){
                bmpInformation.size = sizeof(bmpInfo) + sizeof(colorTable);
                fileHeader.offsetData = sizeof (fileHeader) + sizeof(bmpInfo)+ sizeof(colorTable);
            }
            else{
                bmpInformation.size = sizeof(bmpInfo);
                fileHeader.offsetData = sizeof (fileHeader) + sizeof(bmpInfo);
            }

            fileHeader.fileSize = fileHeader.offsetData;

            if (bmpInformation.height<0) {
                throw std::runtime_error("ERROR! --- Programe can't understand such file");
            }

            data.resize(bmpInformation.height*bmpInformation.width*bmpInformation.bitCount/8);

            if (bmpInformation.width % 4 == 0){
                readFile.read(reinterpret_cast<char*>(data.data()),data.size());
                fileHeader.fileSize += static_cast<uint32_t>(data.size());
            }
            else{
                line = bmpInformation.width*bmpInformation.bitCount / 8;
                uint32_t fullLine = makeFull();
                std::vector<uint8_t> zerroLine(fullLine-line);
                for (int i = 0; i < bmpInformation.height; ++i) {
                    readFile.read(reinterpret_cast<char *>(data.data() + line * i), line);
                    readFile.read(reinterpret_cast<char *>(zerroLine.data()), zerroLine.size());
                }
                fileHeader.fileSize += data.size() + bmpInformation.height*zerroLine.size();
            }
        }
        else{
            throw std::runtime_error("ERROR! --- Can't open this file");
        }
    }
    void write(const char *fileName){
        std::ofstream writeFile(fileName,std::ios_base::binary);
        if (writeFile){
            if (bmpInformation.bitCount == 32){
                writeAll(writeFile);
            }
            else if(bmpInformation.bitCount == 24) {
                if (bmpInformation.bitCount %4 ==0){
                    writeAll(writeFile);
                }
            }
            else{
                uint32_t fullLine = makeFull();
                std::vector<uint8_t> zerroLine(fullLine-line);
                writeHeadInfo(writeFile);
                for (int i = 0; i < bmpInformation.height; ++i) {
                    writeFile.write(reinterpret_cast<char *>(data.data() + line * i), line);
                    writeFile.write(reinterpret_cast<char *>(zerroLine.data()), zerroLine.size());
                }
            }
        }
        else{
            throw std::runtime_error("ERROR! --- Can't open output file ");
        }
    }

    friend void Painter(BMP &bmp1, BMP &bmp2, uint32_t x0, uint32_t y0 );

private:
    uint32_t line{ 0 };
    void checkColor(colorTable &colorInfo){

        colorTable needColor;
        if (needColor.redMask != colorInfo.redMask || needColor.greenMask != colorInfo.greenMask||
        needColor.blueMask != colorInfo.blueMask || needColor.alphaMask != colorInfo.alphaMask){
            throw std::runtime_error("ERROR! --- Unexepted color mask");
        }
        if (needColor.colorSpaceType != colorInfo.colorSpaceType){
            throw std::runtime_error("ERROR! --- Unexepted color space");
        }
    }
    void writeHeadInfo(std::ofstream &writeFile){
        writeFile.write(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
        writeFile.write(reinterpret_cast<char*>(&bmpInformation), sizeof(bmpInformation));
        if (bmpInformation.bitCount == 32){
            writeFile.write(reinterpret_cast<char*>(&bmpColorInfo), sizeof(bmpColorInfo));
        }
    };

    void writeAll(std::ofstream &writeFile){
        writeHeadInfo(writeFile);
        writeFile.write(reinterpret_cast<char*>(data.data()), data.size());
    };

    uint32_t makeFull(uint32_t dop = 4){
        uint32_t fullLine = line;
        while (fullLine % 4 != 0 ){
            fullLine++;
        }
        return fullLine;
    }

};

void Painter(BMP &bmp1, BMP &bmp2, uint32_t x0, uint32_t y0, const char* filename ){
    if(x0+bmp2.bmpInformation.width > bmp1.bmpInformation.width || y0+bmp2.bmpInformation.height > bmp1.bmpInformation.height){
        throw std::runtime_error("The second picture is too bigger");
    }
    uint32_t size1 = bmp1.bmpInformation.bitCount / 8;
    uint32_t size2 = bmp2.bmpInformation.bitCount / 8;
    if (size1 != size2){
        throw std::runtime_error("ERROR! --- Different bits pure pixel in  bmp files");
    }
    else{
        for (uint32_t y = y0; y < y0+bmp2.bmpInformation.height; ++y ){
            for (uint32_t x = x0;x <x0+bmp2.bmpInformation.height;++x){
                bmp1.data[size1*(y*bmp1.bmpInformation.width+x)+0]=bmp2.data[size2*((y-y0)*bmp2.bmpInformation.width+(x-x0))+0];
                bmp1.data[size1*(y*bmp1.bmpInformation.width+x)+1]=bmp2.data[size2*((y-y0)*bmp2.bmpInformation.width+(x-x0))+1];
                bmp1.data[size1*(y*bmp1.bmpInformation.width+x)+2]=bmp2.data[size2*((y-y0)*bmp2.bmpInformation.width+(x-x0))+2];
                if (size1==4){
                    bmp1.data[size1*(y*bmp1.bmpInformation.width+x)+4]=bmp2.data[size2*((y-y0)*bmp2.bmpInformation.width+(x-x0))+4];
                }
            }
        }
        bmp1.write(filename);
    }


}

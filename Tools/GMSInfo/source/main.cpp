/**
 GMS Tool for Hitman Blood Money
 **/
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <cstdio>

#include <array>
#include <string>
#include <unordered_map>

extern "C" {
#include "zlib.h"
}

#include "GlacierTypeDefs.h"

#include "nlohmann/json.hpp"
#include <fmt/format.h>

#define GLACIER_GMS_ZLIB_WBTS -15

struct GMS
{
    int field_0;
    int m_raw;
    int field_8;
    int field_C;
    int field_10;
    int field_14;
};

struct GMSFileEntry
{
    int field_0;
    int field_4;
    int field_8;
};

std::unordered_map<std::string, std::array<std::string, 2>> g_typeInfoDB;

void loadTypesDataBase()
{
    FILE* fp = fopen("typeids.json", "r");
    if (!fp)
    {
        printf("Failed to load typeids.json! All types will not be undecorated!\n");
        return;
    }

    fseek(fp, 0L, SEEK_END);
    size_t bufferSize = ftell(fp);
    rewind(fp);
    char* buffer = (char*)calloc(1, bufferSize);
    fread(buffer, bufferSize, 1, fp);
    fclose(fp);

    std::string sv(buffer, bufferSize);

    /// parse json
    try
    {
        nlohmann::json j = nlohmann::json::parse(sv);
        nlohmann::adl_serializer<decltype(g_typeInfoDB)>::from_json(j, g_typeInfoDB);
        printf("Type Information Database loaded!\n");
    }
    catch (nlohmann::json::exception& ex)
    {
        printf("Bad typeids.json file! JSON Error: %s\n", ex.what());
        throw;
    }
}

std::string getEntityTypeAsStringIfItPossible(unsigned int typeIndex)
{
    std::string value = fmt::format("0x{:X}", typeIndex);
    auto iter = g_typeInfoDB.find(value);
    if (iter != std::end(g_typeInfoDB))
        return fmt::format("{} : {} (0x{:X})", iter->second[0], iter->second[1], typeIndex);
    return fmt::format("NOT FOUND 0x{:X}", typeIndex);
}

void analyzeGMS(const char* prmFilePath, char* buffer, size_t size)
{
    FILE* fp = fopen(prmFilePath, "rb");
    if (!fp)
    {
        printf("Analyze failed! Failed to open PRM file %s\n", prmFilePath);
        return;
    }

    fseek(fp, 0L, SEEK_END);
    auto prmBufferSize = ftell(fp);
    rewind(fp);

    char* prmBuffer = (char*)calloc(1, prmBufferSize);
    fread(prmBuffer, prmBufferSize, 1, fp);
    fclose(fp);

    int totalEntitiesCount = *(int*)&buffer[*(int*)buffer];
    char* result = 0;
    int entityIndex = 0;
    int v5;
    bool v6;
    char* v7;

    if (totalEntitiesCount)
    {
        {
#define G_AT(x) (*(((int*)buffer) + x))
            printf("       +0x10       +0x11       \n");
            printf("       %X          %X          \n", G_AT(0x10), G_AT(0x11));
#undef G_AT
        }

        printf("GMS Import table\n");
        printf("-----------------\n\n");
        printf("Total entities count: %.8d\n", totalEntitiesCount);
        printf("Index |    Ref    |Pointer | Type ID\n");

        while (1)
        {
            v5 = *(int*)(&buffer[8 * entityIndex + 4] + *(int*)buffer) & 0xFFFFFF;
            unsigned int entityType = *(int*)(&buffer[4 * v5 + 20]);
            const bool isLoaderContents = entityType == Glacier::TypeId::ZLoader_Sequence_Setup_ZSTDOBJ;

            int ptr = (int)&buffer[4 * v5];
            std::string typeInfoStr = getEntityTypeAsStringIfItPossible(entityType);

            printf("#%.4d | %.9X | %.8X | %s\n", entityIndex, v5, ptr, typeInfoStr.c_str());

            if (isLoaderContents)
            {
                int addr = (int)&prmBuffer[*((int*)ptr + 3)];
                //TODO: Save XML
            }

            if (++entityIndex == totalEntitiesCount)
            {
                printf("--------------- END OF GMS TABLE ---------------\n");
                break;
            }
        }

        //result = &prmBuffer[*((int*)v7 + 3)];
    }

    free(prmBuffer);
}

bool GMS_Decompress(GMS* gms, char* gmsDecompressed, int gmsInLength)
{
    if (gms->field_14 == 1)
    {
        const int length = (gms->field_C >= gmsInLength) ? gms->field_C : gmsInLength;
        memcpy(gmsDecompressed, (void*)(gms->m_raw + 9), length);
        return true;
    }

    auto entry = reinterpret_cast<GMSFileEntry*>(gms->m_raw);

    z_stream stream;
    stream.avail_in = gms->field_8;
    stream.next_in = (unsigned char*)&entry->field_8 + 1;
    stream.next_out = (Bytef*)gmsDecompressed;
    stream.avail_out = gmsInLength;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;

    if (inflateInit2(&stream, -15) != Z_OK)
    {
        printf("inflateInit2() failed!\n");
        return false;
    }

    int result = inflate(&stream, Z_FINISH);
    inflateEnd(&stream);

    if ((result != Z_STREAM_END || result != Z_BUF_ERROR) && !stream.avail_out)
    {
        printf("inflate() failed with error code %d\n", result);
        return false;
    }

    return true;
}


int main(int argc, char** argv)
{
    if (argc < 3)
    {
        printf("ERROR: You need to specify input and output files!\n\tUsage: gmstool.exe <GMS File> <PRM File> [Path to new uncompressed GMS]\n");
        return 0;
    }

    const char* inFile    = argv[1];
    const char* prmInFile = argv[2];

    printf(" *** GMS TOOL ***\n");
    printf(" * IN  : %s\n", inFile);

    FILE* fp = fopen(inFile, "rb");
    if (!fp)
    {
        printf("Failed to open file %s\n", inFile);
        return -400;
    }

    fseek(fp, 0L, SEEK_END);
    auto rawBufferSize = ftell(fp);
    rewind(fp);

    char* rawBuffer = (char*)malloc(rawBufferSize);
    auto readyBytes = fread(rawBuffer, sizeof(char), rawBufferSize, fp);

    if (rawBufferSize != readyBytes)
    {
        printf("Failed to read full contents from raw stream! Required %d ready %d\n", rawBufferSize, readyBytes);
        fclose(fp);
        return -1;
    }
    fclose(fp);

    // decompress
    GMS gms = { 0 };
    gms.field_0 = 1;
    gms.m_raw = (int)rawBuffer;

    int v5 = *(unsigned long*)rawBuffer;

    gms.field_C = v5;
    gms.field_8 = *(unsigned long*)(rawBuffer + 4);
    gms.field_14 = (*(unsigned char*)(rawBuffer + 8)) != 0;

    int outBuffSize = (v5 + 15) & 0xFFFFFFF0;
    char* outBuffer = (char*)malloc(outBuffSize);
    memset(outBuffer, 0x0, outBuffSize);
    GMS_Decompress(&gms, outBuffer, outBuffSize);

    /*
    if (argv > 3) {
        const char* outFile = argv[3];
        printf(" * OUT : %s\n", outFile);

        printf(" *** DUMP ***\n");
        printf(" Decompressed size: 0x%.8X (%d)\n", gms.field_C, gms.field_C);
        for (int i = 0; i < outBuffSize; i += 4)
        {
            printf("%.4X %.2X %.2X %.2X %.2X\n", i, outBuffer[i], outBuffer[i + 1], outBuffer[i + 2], outBuffer[i + 3]);
        }

        FILE* outFP = fopen(outFile, "a+");
        if (outFP)
        {
            fwrite(outBuffer, sizeof(char), outBuffSize, outFP);
            fclose(outFP);
        }
        else
        {
            printf("Failed to open file %s to write result!\n", outFile);
        }

        printf(" --- PARSING DONE ---\n");
    }
    */

    loadTypesDataBase();
    analyzeGMS(prmInFile, outBuffer, outBuffSize);

    free(rawBuffer);
    free(outBuffer);

    return 0;
}
/*
MIT License

Copyright (c) 2022 Clay Paky S.R.L.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#include "Factories/CPGDTFUnzip.h"
#include "ClayPakyGDTFImporterLog.h"
#include "Libs/MiniZ/miniz.h"


/**
    * Extract any type of file from the GDTF archive
    * @author Dorian Gardes - Clay Paky S.R.L.
    * @date 29 april 2022
    *
    * @return void* => buffer readed
    * @return  int  => buffer size
    */
std::tuple<void*, int> UCPGDTFUnzip::ExtractFileFromGDTFArchive(const FString& GDTFFullPath, const FString Filename) {

    mz_zip_archive zip_archive; // Struct containing the zip archive metadata in memory
    memset(&zip_archive, 0, sizeof(zip_archive)); // Setup of the memory

    //Opening of the zip file.
    if (!mz_zip_reader_init_file(&zip_archive, TCHAR_TO_ANSI(*GDTFFullPath), 0)) {
        UE_LOG_CPGDTFIMPORTER(Warning, TEXT("Failed to open GDTF file : %s"), mz_zip_get_error_string(mz_zip_get_last_error(&zip_archive)));
        mz_zip_reader_end(&zip_archive);
        return std::tuple<void*, int> {nullptr, 0};
    }

    // Looking for the file inside the archive
    int file_index = mz_zip_reader_locate_file(&zip_archive, TCHAR_TO_ANSI(*Filename), NULL, 0);
    if (file_index == -1) {
        UE_LOG_CPGDTFIMPORTER(Warning, TEXT("File %s not found in GDTF"), *Filename);
        mz_zip_reader_end(&zip_archive);
        return std::tuple<void*, int> {nullptr, 0};
    }

    // Reading the file
    void* fileContent = nullptr;
    size_t fileContentSize = 0;
    fileContent = mz_zip_reader_extract_to_heap(&zip_archive, file_index, &fileContentSize, NULL);
    if (!fileContent) {
        UE_LOG_CPGDTFIMPORTER(Warning, TEXT("Failed to extract %s file : %s"), *Filename, mz_zip_get_error_string(mz_zip_get_last_error(&zip_archive)));
        mz_zip_reader_end(&zip_archive);
        return std::tuple<void*, int> {nullptr, 0};
    }

    mz_zip_reader_end(&zip_archive);
    return std::tuple<void*, int> {fileContent, fileContentSize};
}

/**
    * Extract text file from the GDTF archive
    * @author Dorian Gardes - Clay Paky S.R.L.
    * @date 29 april 2022
    */
FString UCPGDTFUnzip::ExtractTextFileFromGDTFArchive(const FString& GDTFFullPath, const FString Filename) {

    // Extraction with generic function
    std::tuple<void*, int> bufferTuple = ExtractFileFromGDTFArchive(GDTFFullPath, Filename);
    if (!std::get<0>(bufferTuple)) { // Check if read problems
        UE_LOG_CPGDTFIMPORTER(Warning, TEXT("Failed to extract %s file from GDTF archive."), *Filename);
        return FString();
    }
    else {
        FString FileString;
        // Conversion to real string to remove strange characters at the end of the void* buffer
        FileString.Append((char*)std::get<0>(bufferTuple), std::get<1>(bufferTuple));
        free(std::get<0>(bufferTuple)); // Free memory
        return FileString;
    }
}
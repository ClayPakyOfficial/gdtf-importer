/*
MIT License

Copyright (c) 2022 Clay Paky S.P.A.

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


#pragma once
#include "CoreMinimal.h"

/// Helpers to manipulate GDTF archives
class UCPGDTFUnzip {

public:
    /**
     * Extract any type of file from the GDTF archive
     * @author Dorian Gardes - Clay Paky S.P.A.
     * @date 29 april 2022
     *
     * @return void* => buffer readed
     * @return  int  => buffer size
     */
    static std::tuple<void*, int> ExtractFileFromGDTFArchive(const FString& GDTFFullPath, const FString Filename);

    /**
     * Extract text file from the GDTF archive
     * @author Dorian Gardes - Clay Paky S.P.A.
     * @date 29 april 2022
     */
    static FString ExtractTextFileFromGDTFArchive(const FString& GDTFFullPath, const FString Filename);

};



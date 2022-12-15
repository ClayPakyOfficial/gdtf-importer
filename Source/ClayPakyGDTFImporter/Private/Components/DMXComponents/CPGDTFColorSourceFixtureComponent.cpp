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


#include "Components/DMXComponents/CPGDTFColorSourceFixtureComponent.h"

FCPDMXColorChannelData::FCPDMXColorChannelData(FDMXImportGDTFDMXChannel Channel, ECPGDTFAttributeType ColorAttribute_) {

	if (Channel.Offset.Num() > 0) this->Address = FMath::Max(1, FMath::Min(512, Channel.Offset[0]));
	else this->Address = -1;
	this->MinValue = Channel.LogicalChannels[0].ChannelFunctions[0].PhysicalFrom;
	FDMXImportGDTFLogicalChannel LastLogicalChannel = Channel.LogicalChannels[Channel.LogicalChannels.Num() - 1];
	FDMXImportGDTFChannelFunction LastChannelFunction = LastLogicalChannel.ChannelFunctions[LastLogicalChannel.ChannelFunctions.Num() - 1];
	this->MaxValue = LastChannelFunction.PhysicalTo;
	this->DefaultValue = 0.0f;
	this->ColorAttribute = ColorAttribute_;
}


void UCPGDTFColorSourceFixtureComponent::BeginPlay() {

	Super::BeginPlay();
	this->bUseInterpolation = false;
	this->CurrentColor = FLinearColor(0, 0, 0); // Black by default
}
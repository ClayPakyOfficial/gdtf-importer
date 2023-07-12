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

#pragma once

#include "CoreMinimal.h"
#include "CPGDTFDescription.h"
#include "CPGDTFDMXChannelTree.generated.h"

/**
 * Trees to optimize the DMXChannel definition during runtime
 */

/**
 * Node containing FDMXImportGDTFChannelSet
 */
USTRUCT()
struct FDMXChannelSetTreeNode {
	GENERATED_BODY()
public:

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Internal")
	int32 Left;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Internal")
	int32 Right;
	
	UPROPERTY()
	FCPGDTFDescriptionChannelSet ChannelSet;

	bool IsValueInRange(int32 DMXValue);

	bool operator<(const int32& Other) const;

	bool operator<(const FDMXChannelSetTreeNode& Other) const;
	bool operator>(const FDMXChannelSetTreeNode& Other) const;
	bool operator<=(const FDMXChannelSetTreeNode& Other) const;
	bool operator>=(const FDMXChannelSetTreeNode& Other) const;
};

/**
 * Tree of FDMXImportGDTFChannelSet
 */
USTRUCT(Blueprinttype)
struct FDMXLogicalChannelTree {
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "Internal")
	int32 Root = -1;

	UPROPERTY(VisibleDefaultsOnly, Category = "Internal")
	TArray<FDMXChannelSetTreeNode> elements;
public:

	bool IsEmpty() { return elements.IsEmpty(); }

	void Insert(FCPGDTFDescriptionChannelFunction Item);
	
	FCPGDTFDescriptionChannelSet* GetChannelSetByDMXValue(int32 DMXValue);

protected:
	
	void Insert(FCPGDTFDescriptionChannelSet& Item);
};


/**
 * Node containing a FDMXImportGDTFLogicalChannel
 */
USTRUCT()
struct FDMXChannelTreeNode {
	GENERATED_BODY()
public:

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Internal")
	int32 Left = -1;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Internal")
	int32 Right = -1;

	UPROPERTY()
	FDMXLogicalChannelTree ChannelSetsTree;
	UPROPERTY()
	FCPGDTFDescriptionChannelFunction ChannelFunction;

	bool IsValueInRange(int32 DMXValue);

	bool operator<(const int32& Other) const;

	bool operator<(const FDMXChannelTreeNode& Other) const;
	bool operator>(const FDMXChannelTreeNode& Other) const;
	bool operator<=(const FDMXChannelTreeNode& Other) const;
	bool operator>=(const FDMXChannelTreeNode& Other) const;
};

/**
 * Tree of FCPGDTFChannelFunction
 */
USTRUCT(BlueprintType)
struct FDMXChannelTree {
	GENERATED_BODY()
private:
	UPROPERTY(VisibleDefaultsOnly, Category = "Internal")
	int32 Root = -1;

	UPROPERTY(VisibleDefaultsOnly, Category = "Internal")
	TArray<FDMXChannelTreeNode> elements;

public:

	bool IsEmpty() { return elements.IsEmpty(); }

	/**
	 * Insert a logical Channel in the Tree
	 * @author Dorian Gardes - Clay Paky S.R.L.
	 * 
	 * @param Item
	 * @param NbrDMXChannels param is the number of DMXChannels used for this LogicalChannel.
	 * Typically it is the FDMXImportGDTFDMXChannel->Offset.Num()
	 */
	void Insert(FDMXImportGDTFLogicalChannel Item, uint8 NbrDMXChannels);
	
	TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*> GetBehaviourByDMXValue(int32 DMXValue);

protected:

	void Insert(FCPGDTFDescriptionChannelFunction& Item);
};
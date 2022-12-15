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
#include "CPGDTFDescription.h"

/**
 * Trees to optimize the DMXChannel definition during runtime
 */

/**
 * Node containing FDMXImportGDTFChannelSet
 */
struct FDMXChannelSetTreeNode {

public:

	FDMXChannelSetTreeNode* Left;
	FDMXChannelSetTreeNode* Right;
	
	FCPGDTFDescriptionChannelSet* ChannelSet;

	~FDMXChannelSetTreeNode() {
		
		if (this->Left  != nullptr) this->Left->~FDMXChannelSetTreeNode();
		if (this->Right != nullptr) this->Right->~FDMXChannelSetTreeNode();
	}

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
class FDMXLogicalChannelTree {

protected:
	FDMXChannelSetTreeNode* Root;

public:
	FDMXLogicalChannelTree() { this->Root = nullptr; }

	~FDMXLogicalChannelTree() { if (this->Root != nullptr) this->Root->~FDMXChannelSetTreeNode(); }

	bool IsEmpty() { return this->Root == nullptr; }

	void Insert(FCPGDTFDescriptionChannelFunction Item);
	
	FCPGDTFDescriptionChannelSet* GetChannelSetByDMXValue(int32 DMXValue);

protected:
	
	void Insert(FCPGDTFDescriptionChannelSet* Item);
};


/**
 * Node containing a FDMXImportGDTFLogicalChannel
 */
struct FDMXChannelTreeNode {

public:

	FDMXChannelTreeNode* Left;
	FDMXChannelTreeNode* Right;

	FDMXLogicalChannelTree ChannelSetsTree;
	FCPGDTFDescriptionChannelFunction* ChannelFunction;

	~FDMXChannelTreeNode() {

		if (this->Left  != nullptr) this->Left->~FDMXChannelTreeNode();
		if (this->Right != nullptr) this->Right->~FDMXChannelTreeNode();
		this->ChannelSetsTree.~FDMXLogicalChannelTree();
	}

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
class FDMXChannelTree {

public:
	FDMXChannelTreeNode* Root;

public:
	FDMXChannelTree() { Root = nullptr; }

	~FDMXChannelTree() {
		if (this->Root != nullptr) this->Root->~FDMXChannelTreeNode();
	}

	bool IsEmpty() { return this->Root == nullptr; }

	/**
	 * Insert a logical Channel in the Tree
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * 
	 * @param Item
	 * @param NbrDMXChannels param is the number of DMXChannels used for this LogicalChannel.
	 * Typically it is the FDMXImportGDTFDMXChannel->Offset.Num()
	 */
	void Insert(FDMXImportGDTFLogicalChannel Item, uint8 NbrDMXChannels);
	
	TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*> GetBehaviourByDMXValue(int32 DMXValue);

protected:

	void Insert(FCPGDTFDescriptionChannelFunction* Item);
};
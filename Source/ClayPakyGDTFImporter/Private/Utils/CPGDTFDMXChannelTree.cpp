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

#include "CPGDTFDMXChannelTree.h"

/*************************************************************/

bool FDMXChannelSetTreeNode::IsValueInRange(int32 DMXValue) {

    // Permit to deal with value 255 without edit GDTF values for a possible future export
    if (DMXValue == 255) return DMXValue >= this->ChannelSet->DMXFrom.Value && DMXValue <= this->ChannelSet->DMXTo.Value;
    else return DMXValue >= this->ChannelSet->DMXFrom.Value && DMXValue < this->ChannelSet->DMXTo.Value;
}

bool FDMXChannelSetTreeNode::operator<(const int32& Other) const {
    return this->ChannelSet->DMXTo.Value <= Other;
}

bool FDMXChannelSetTreeNode::operator<(const FDMXChannelSetTreeNode& Other) const {
    return this->ChannelSet->DMXTo.Value < Other.ChannelSet->DMXTo.Value;
}

bool FDMXChannelSetTreeNode::operator>(const FDMXChannelSetTreeNode& Other) const {
    return this->ChannelSet->DMXFrom.Value > Other.ChannelSet->DMXFrom.Value;
}

bool FDMXChannelSetTreeNode::operator<=(const FDMXChannelSetTreeNode& Other) const {
    return this->ChannelSet->DMXTo.Value <= Other.ChannelSet->DMXTo.Value;
}

bool FDMXChannelSetTreeNode::operator>=(const FDMXChannelSetTreeNode& Other) const {
    return this->ChannelSet->DMXFrom.Value >= Other.ChannelSet->DMXFrom.Value;
}

/*************************************************************/

void FDMXLogicalChannelTree::Insert(FCPGDTFDescriptionChannelFunction Item) {
    
    for (int i=0; i < Item.ChannelSets.Num(); i++) {

        if (i == Item.ChannelSets.Num() -1) this->Insert(new FCPGDTFDescriptionChannelSet(Item.ChannelSets[i], Item.DMXTo));
        else this->Insert(new FCPGDTFDescriptionChannelSet(Item.ChannelSets[i], Item.ChannelSets[i+1].DMXFrom));
    }
}

void FDMXLogicalChannelTree::Insert(FCPGDTFDescriptionChannelSet* Item) {

    FDMXChannelSetTreeNode* NewElement = new FDMXChannelSetTreeNode();
    FDMXChannelSetTreeNode* Parent;
    NewElement->ChannelSet = Item;
    NewElement->Left = nullptr;
    NewElement->Right = nullptr;
    Parent = nullptr;

    if (this->IsEmpty()) Root = NewElement;
    else {

        FDMXChannelSetTreeNode* Ptr = Root;
        while (Ptr != nullptr) {
            Parent = Ptr;
            if (*NewElement > *Ptr) Ptr = Ptr->Right;
            else Ptr = Ptr->Left;
        }
        if (*NewElement < *Parent) Parent->Left = NewElement;
        else Parent->Right = NewElement;
    }
}

FCPGDTFDescriptionChannelSet* FDMXLogicalChannelTree::GetChannelSetByDMXValue(int32 DMXValue) {

    if (this->IsEmpty()) return nullptr;

    FDMXChannelSetTreeNode* CurrentNode = this->Root;

    while (!CurrentNode->IsValueInRange(DMXValue)) {

        //go to right tree
        if (*CurrentNode < DMXValue) CurrentNode = CurrentNode->Right;
        //else go to left tree
        else CurrentNode = CurrentNode->Left;
        if (CurrentNode == nullptr) return nullptr;
    }
    return CurrentNode->ChannelSet;
}

/*************************************************************/

bool FDMXChannelTreeNode::IsValueInRange(int32 DMXValue) {

    // Permit to deal with value 255 without edit GDTF values for a possible future export
    if (DMXValue == 255) return DMXValue >= this->ChannelFunction->DMXFrom.Value && DMXValue <= this->ChannelFunction->DMXTo.Value;
    else return DMXValue >= this->ChannelFunction->DMXFrom.Value && DMXValue < this->ChannelFunction->DMXTo.Value;
}

bool FDMXChannelTreeNode::operator<(const int32& Other) const {
    return this->ChannelFunction->DMXTo.Value <= Other;
}

bool FDMXChannelTreeNode::operator<(const FDMXChannelTreeNode& Other) const {
    return this->ChannelFunction->DMXTo.Value < Other.ChannelFunction->DMXTo.Value;
}

bool FDMXChannelTreeNode::operator>(const FDMXChannelTreeNode& Other) const {
    return this->ChannelFunction->DMXFrom.Value > Other.ChannelFunction->DMXFrom.Value;
}

bool FDMXChannelTreeNode::operator<=(const FDMXChannelTreeNode& Other) const {
    return this->ChannelFunction->DMXTo.Value <= Other.ChannelFunction->DMXTo.Value;
}

bool FDMXChannelTreeNode::operator>=(const FDMXChannelTreeNode& Other) const {
    return this->ChannelFunction->DMXFrom.Value >= Other.ChannelFunction->DMXFrom.Value;
}

/*************************************************************/

void FDMXChannelTree::Insert(FDMXImportGDTFLogicalChannel Item, uint8 NbrDMXChannels) {

    for (int i = 0; i < Item.ChannelFunctions.Num(); i++) {
    
        if (i == Item.ChannelFunctions.Num() -1) { // If this is the last ChannelFunction

            FDMXImportGDTFDMXValue EndChannelValue; // We calc the value of the end of the channel
            EndChannelValue.Value = FMath::Pow(256.0f, NbrDMXChannels) - 1;
            EndChannelValue.ValueSize = NbrDMXChannels;

            this->Insert(new FCPGDTFDescriptionChannelFunction(Item.ChannelFunctions[i], EndChannelValue));
        } else this->Insert(new FCPGDTFDescriptionChannelFunction(Item.ChannelFunctions[i], Item.ChannelFunctions[i+1].DMXFrom));
    }   
}

void FDMXChannelTree::Insert(FCPGDTFDescriptionChannelFunction* Item) {

    FDMXChannelTreeNode* NewElement = new FDMXChannelTreeNode();
    FDMXChannelTreeNode* Parent;
    NewElement->ChannelFunction = Item;
    NewElement->Left = nullptr;
    NewElement->Right = nullptr;
    NewElement->ChannelSetsTree = FDMXLogicalChannelTree();
    NewElement->ChannelSetsTree.Insert(*Item);
    Parent = nullptr;

    if (this->IsEmpty()) Root = NewElement;
    else {

        FDMXChannelTreeNode* Ptr = Root;
        while (Ptr != nullptr) {
            Parent = Ptr;
            if (*NewElement > *Ptr) Ptr = Ptr->Right;
            else Ptr = Ptr->Left;
        }
        if (*NewElement < *Parent) Parent->Left = NewElement;
        else Parent->Right = NewElement;
    }
}

TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*> FDMXChannelTree::GetBehaviourByDMXValue(int32 DMXValue) {

    if (this->IsEmpty()) return {nullptr, nullptr};

    FDMXChannelTreeNode* ChannelFunctionNode = this->Root;

    while (!ChannelFunctionNode->IsValueInRange(DMXValue)) {

        //go to right tree
        if (*ChannelFunctionNode < DMXValue) ChannelFunctionNode = ChannelFunctionNode->Right;
        //else go to left tree
        else ChannelFunctionNode = ChannelFunctionNode->Left;

        if (ChannelFunctionNode == nullptr) return {nullptr, nullptr};
    }

    TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*> ReturnTuple;
    ReturnTuple.Key = ChannelFunctionNode->ChannelFunction;
    ReturnTuple.Value = ChannelFunctionNode->ChannelSetsTree.GetChannelSetByDMXValue(DMXValue);

    return ReturnTuple;
}
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

#include "CPGDTFDMXChannelTree.h"
/*************************************************************/

bool FDMXChannelSetTreeNode::IsValueInRange(int32 DMXValue) {

    // Permit to deal with value 255 without edit GDTF values for a possible future export
    if (DMXValue == 0xff || DMXValue == 0xffff || DMXValue == 0xffffff || DMXValue == 0xffffffff) {
        return DMXValue >= this->ChannelSet.DMXFrom.Value && DMXValue <= this->ChannelSet.DMXTo.Value;
    } else return DMXValue >= this->ChannelSet.DMXFrom.Value && DMXValue < this->ChannelSet.DMXTo.Value;
}

bool FDMXChannelSetTreeNode::operator<(const int32& Other) const {
    return this->ChannelSet.DMXTo.Value <= Other;
}

bool FDMXChannelSetTreeNode::operator<(const FDMXChannelSetTreeNode& Other) const {
    return this->ChannelSet.DMXTo.Value < Other.ChannelSet.DMXTo.Value;
}

bool FDMXChannelSetTreeNode::operator>(const FDMXChannelSetTreeNode& Other) const {
    return this->ChannelSet.DMXFrom.Value > Other.ChannelSet.DMXFrom.Value;
}

bool FDMXChannelSetTreeNode::operator<=(const FDMXChannelSetTreeNode& Other) const {
    return this->ChannelSet.DMXTo.Value <= Other.ChannelSet.DMXTo.Value;
}

bool FDMXChannelSetTreeNode::operator>=(const FDMXChannelSetTreeNode& Other) const {
    return this->ChannelSet.DMXFrom.Value >= Other.ChannelSet.DMXFrom.Value;
}

/*************************************************************/

void FDMXLogicalChannelTree::Insert(FCPGDTFDescriptionChannelFunction Item) {
    
    for (int i=0; i < Item.ChannelSets.Num(); i++) {

        if (i == Item.ChannelSets.Num() - 1) {
            auto elem = FCPGDTFDescriptionChannelSet(Item.ChannelSets[i], Item.DMXTo);
            this->Insert(elem);
        } else {
            auto elem = FCPGDTFDescriptionChannelSet(Item.ChannelSets[i], Item.ChannelSets[i + 1].DMXFrom);
            this->Insert(elem);
        }
    }
}

void FDMXLogicalChannelTree::Insert(FCPGDTFDescriptionChannelSet& Item) {

    FDMXChannelSetTreeNode NewElement = FDMXChannelSetTreeNode();
    FDMXChannelSetTreeNode* Parent = nullptr;
    NewElement.ChannelSet = Item;
    NewElement.Left = -1;
    NewElement.Right = -1;


    if (this->IsEmpty()) Root = elements.Add(NewElement);
    else {
        int32 elementId = elements.Add(NewElement);

        int32 idx = Root;
        while (idx != -1) {
            Parent = &elements[idx];
            if (NewElement > *Parent) idx = Parent->Right;
            else idx = Parent->Left;
        }
        if (NewElement < *Parent) Parent->Left = elementId;
        else Parent->Right = elementId;
    }
}

FCPGDTFDescriptionChannelSet* FDMXLogicalChannelTree::GetChannelSetByDMXValue(int32 DMXValue) {
    if (this->IsEmpty()) return nullptr;
    FDMXChannelSetTreeNode* CurrentNode = &elements[this->Root];

    while (!CurrentNode->IsValueInRange(DMXValue)) {
        int32 index = *CurrentNode < DMXValue ? CurrentNode->Right : CurrentNode->Left;
        if (index < 0) return nullptr;
        CurrentNode = &elements[index];
    }
    return &CurrentNode->ChannelSet;
}

/*************************************************************/

bool FDMXChannelTreeNode::IsValueInRange(int32 DMXValue) {

    // Permit to deal with value 255 without edit GDTF values for a possible future export
    if (DMXValue == 0xff || DMXValue == 0xffff || DMXValue == 0xffffff || DMXValue == 0xffffffff) {
        return DMXValue >= this->ChannelFunction.DMXFrom.Value && DMXValue <= this->ChannelFunction.DMXTo.Value;
    } else return DMXValue >= this->ChannelFunction.DMXFrom.Value && DMXValue < this->ChannelFunction.DMXTo.Value;
}

bool FDMXChannelTreeNode::operator<(const int32& Other) const {
    return this->ChannelFunction.DMXTo.Value <= Other;
}

bool FDMXChannelTreeNode::operator<(const FDMXChannelTreeNode& Other) const {
    return this->ChannelFunction.DMXTo.Value < Other.ChannelFunction.DMXTo.Value;
}

bool FDMXChannelTreeNode::operator>(const FDMXChannelTreeNode& Other) const {
    return this->ChannelFunction.DMXFrom.Value > Other.ChannelFunction.DMXFrom.Value;
}

bool FDMXChannelTreeNode::operator<=(const FDMXChannelTreeNode& Other) const {
    return this->ChannelFunction.DMXTo.Value <= Other.ChannelFunction.DMXTo.Value;
}

bool FDMXChannelTreeNode::operator>=(const FDMXChannelTreeNode& Other) const {
    return this->ChannelFunction.DMXFrom.Value >= Other.ChannelFunction.DMXFrom.Value;
}

/*************************************************************/

void FDMXChannelTree::Insert(FDMXImportGDTFLogicalChannel Item, uint8 NbrDMXChannels) {

    for (int i = 0; i < Item.ChannelFunctions.Num(); i++) {
    
        if (i == Item.ChannelFunctions.Num() -1) { // If this is the last ChannelFunction

            FDMXImportGDTFDMXValue EndChannelValue; // We calc the value of the end of the channel
            EndChannelValue.Value = FMath::Pow(256.0f, NbrDMXChannels) - 1;
            EndChannelValue.ValueSize = NbrDMXChannels;

            auto elem = FCPGDTFDescriptionChannelFunction(Item.ChannelFunctions[i], EndChannelValue);
            this->Insert(elem);
        } else {
            auto elem = FCPGDTFDescriptionChannelFunction(Item.ChannelFunctions[i], Item.ChannelFunctions[i + 1].DMXFrom);
            this->Insert(elem);
        }
    }   
}

void FDMXChannelTree::Insert(FCPGDTFDescriptionChannelFunction& Item) {

    FDMXChannelTreeNode NewElement = FDMXChannelTreeNode();
    FDMXChannelTreeNode *Parent = nullptr;
    NewElement.ChannelFunction = Item;
    NewElement.Left = -1;
    NewElement.Right = -1;
    NewElement.ChannelSetsTree = FDMXLogicalChannelTree();
    NewElement.ChannelSetsTree.Insert(Item);


    if (this->IsEmpty()) Root = elements.Add(NewElement);
    else {
        int32 elementId = elements.Add(NewElement);

        int32 idx = Root;
        while (idx != -1) {
            Parent = &elements[idx];
            if (NewElement > *Parent) idx = Parent->Right;
            else idx = Parent->Left;
        }
        if (NewElement < *Parent) Parent->Left = elementId;
        else Parent->Right = elementId;
    }
}

TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*> FDMXChannelTree::GetBehaviourByDMXValue(int32 DMXValue) {
    if (this->IsEmpty()) return {nullptr, nullptr};
    FDMXChannelTreeNode* ChannelFunctionNode = &elements[this->Root];

    while (!ChannelFunctionNode->IsValueInRange(DMXValue)) {
        int32 index = *ChannelFunctionNode < DMXValue ? ChannelFunctionNode->Right : ChannelFunctionNode->Left;
        if (index < 0) return {nullptr, nullptr};
        ChannelFunctionNode = &elements[index];
    }

    TTuple<FCPGDTFDescriptionChannelFunction*, FCPGDTFDescriptionChannelSet*> ReturnTuple;
    ReturnTuple.Key = &ChannelFunctionNode->ChannelFunction;
    ReturnTuple.Value = ChannelFunctionNode->ChannelSetsTree.GetChannelSetByDMXValue(DMXValue);

    return ReturnTuple;
}
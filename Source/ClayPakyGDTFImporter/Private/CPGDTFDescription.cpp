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

#include "CPGDTFDescription.h"

/*
 * Parses a GDTF Matrix
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 24 may 2022
 *
 * @param MatrixStr String to parse into a matrix
 * @returns FMatrix
 */
FMatrix CPGDTFDescription::ParseMatrix(FString MatrixStr) {

	// MatrixStr looks like this:
	// {1.000000,2.000000,3.000000,4.000000}{5.000000,6.000000,7.000000,8.000000}{9.000000,10.000000,11.000000,12.000000}{13,14,15,16}

	FMatrix Matrix = FMatrix(EForceInit::ForceInitToZero);

	if (MatrixStr.IsEmpty()) return Matrix;

	MatrixStr.RemoveAt(0); // We remove the '{' at the begining of the string
	MatrixStr.RemoveAt(MatrixStr.Len() - 1); // We remove the '}' at the end of the string

	TArray<FString> RowsStrs;
	MatrixStr.ParseIntoArray(RowsStrs, TEXT("}{")); // We split the rows

	// Loop on the rows
	for (int i = 0; i < RowsStrs.Num(); i++) {

		TArray<FString> CellsStrs;
		RowsStrs[i].ParseIntoArray(CellsStrs, TEXT(",")); // We split the cells

		// Loop on the cells
		for (int j = 0; j < CellsStrs.Num(); j++) {

			// Parse the string to float and fill the matrix
			Matrix.M[i][j] = FCString::Atof(*CellsStrs[j]) * 100.0f; // Multiplication by 100 because the unit on GDTF is the meter.
			// Note: On Unreal the unit is the centimeter.
		}
	}
	return Matrix;
}


/**
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 15 june 2022
 *
 * @param Type
 * @return Childrens
*/
TArray<UCPGDTFDescriptionGeometryBase*> UCPGDTFDescriptionGeometries::GetChildrensOfType(ECPGDTFDescriptionGeometryType Type) {

	TArray<UCPGDTFDescriptionGeometryBase*> Childrens;
	for (UCPGDTFDescriptionGeometryBase* TopGeometry : this->Geometries) {
		for (UCPGDTFDescriptionGeometryBase* SubGeometry : UCPGDTFDescriptionGeometries::GetChildrensOfType_STATIC(TopGeometry, Type)) {
			Childrens.Add(SubGeometry);
		}
	}
	return Childrens;
}

/**
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 15 june 2022
 *
 * @param CurrentNode
 * @param Type
 * @return Childrens of specified node
*/
TArray<UCPGDTFDescriptionGeometryBase*> UCPGDTFDescriptionGeometries::GetChildrensOfType_STATIC(UCPGDTFDescriptionGeometryBase* CurrentNode, ECPGDTFDescriptionGeometryType Type) {

	TArray<UCPGDTFDescriptionGeometryBase*> ChildrensOfCurrentNode;
	for (UCPGDTFDescriptionGeometryBase* SubNode : CurrentNode->Childrens) {

		// 1. We add our childrens of Type specified
		if (SubNode->Type == Type) ChildrensOfCurrentNode.Add(SubNode);

		// 2. We add all the subNodes
		TArray<UCPGDTFDescriptionGeometryBase*> ChildrensOfSubNode = UCPGDTFDescriptionGeometries::GetChildrensOfType_STATIC(SubNode, Type);
		for (UCPGDTFDescriptionGeometryBase* SubSubNode : ChildrensOfSubNode) {
			ChildrensOfCurrentNode.Add(SubSubNode);
		}
	}
	return ChildrensOfCurrentNode;
}

/**
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 15 june 2022
 *
 * @param Type
 * @return Childrens Names
*/
TArray<FName> UCPGDTFDescriptionGeometries::GetChildrensNamesOfType(ECPGDTFDescriptionGeometryType Type) {

	TArray<FName> Names;
	for (UCPGDTFDescriptionGeometryBase* Geometry : UCPGDTFDescriptionGeometries::GetChildrensOfType(Type)) {
		Names.Add(Geometry->Name);
	}
	return Names;
}

/**
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 15 june 2022
 *
 * @param CurrentNode
 * @param Type
 * @return Childrens Names
*/
TArray<FName> UCPGDTFDescriptionGeometries::GetChildrensNamesOfType_STATIC(UCPGDTFDescriptionGeometryBase* CurrentNode, ECPGDTFDescriptionGeometryType Type) {

	TArray<FName> Names;
	for (UCPGDTFDescriptionGeometryBase* Geometry : UCPGDTFDescriptionGeometries::GetChildrensOfType_STATIC(CurrentNode, Type)) {
		Names.Add(Geometry->Name);
	}
	return Names;
}

FString UCPGDTFDescription::GetFixtureSavePath() {

	FString FixtureSavePath = this->GetOuter()->GetFName().ToString();
	if (FixtureSavePath[FixtureSavePath.Len()-1] != '/') {
		int Index;
		FixtureSavePath.FindLastChar('/', Index);
		FixtureSavePath.LeftInline(Index + 1);
	}
	return FixtureSavePath;
}
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


#include "Widgets/UCPGDTFDescriptionThumbnailRenderer.h"
#include "CPGDTFDescription.h"
#include <Editor/UnrealEd/Classes/ThumbnailRendering/ThumbnailManager.h>
#include <Runtime/Engine/Public/CanvasItem.h>
#include <Runtime/Engine/Public/CanvasTypes.h>


void UCPGDTFDescriptionThumbnailRenderer::GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const {
	
	UCPGDTFDescription* Asset = Cast<UCPGDTFDescription>(Object);

	if (Asset) {
		if (UTexture2D* Texture = this->GetTextureFromAsset(Asset)) {
			OutWidth = FMath::TruncToInt(Zoom * (float)Texture->GetSurfaceWidth());
			OutHeight = FMath::TruncToInt(Zoom * (float)Texture->GetSurfaceHeight());
			return;
		}
	}
	Super::GetThumbnailSize(Object, Zoom, OutWidth, OutHeight);
}

void UCPGDTFDescriptionThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily) {

	UCPGDTFDescription* Asset = Cast<UCPGDTFDescription>(Object);

	if (Asset) {
		if (UTexture2D* Texture2D = this->GetTextureFromAsset(Asset)) {

			const bool bUseTranslucentBlend = Texture2D && Texture2D->HasAlphaChannel() && ((Texture2D->LODGroup == TEXTUREGROUP_UI) || (Texture2D->LODGroup == TEXTUREGROUP_Pixels2D));

			TRefCountPtr<FBatchedElementParameters> BatchedElementParameters;
			if (bUseTranslucentBlend) {

				// If using alpha, draw a checkerboard underneath first.
				const int32 CheckerDensity = 8;
				TObjectPtr<UTexture2D> Checker = UThumbnailManager::Get().CheckerboardTexture;
				Canvas->DrawTile(
					0.0f, 0.0f, Width, Height, // Dimensions
					0.0f, 0.0f, CheckerDensity, CheckerDensity, // UVs
					FLinearColor::White, Checker->GetResource()); // Tint & Texture
			}
			// Use A canvas tile item to draw
			FCanvasTileItem CanvasTile(FVector2D(X, Y), Texture2D->GetResource(), FVector2D(Width, Height), FLinearColor::White);
			CanvasTile.BlendMode = bUseTranslucentBlend ? SE_BLEND_Translucent : SE_BLEND_Opaque;
			CanvasTile.BatchedElementParameters = BatchedElementParameters;
			CanvasTile.Draw(Canvas);
			if (Texture2D && Texture2D->IsCurrentlyVirtualTextured())
			{
				auto VTChars = TEXT("VT");
				int32 VTWidth = 0;
				int32 VTHeight = 0;
				StringSize(GEngine->GetLargeFont(), VTWidth, VTHeight, VTChars);
				float PaddingX = Width / 128.0f;
				float PaddingY = Height / 128.0f;
				float ScaleX = Width / 64.0f; //Text is 1/64'th of the size of the thumbnails
				float ScaleY = Height / 64.0f;
				// VT overlay
				FCanvasTextItem TextItem(FVector2D(Width - PaddingX - VTWidth * ScaleX, Height - PaddingY - VTHeight * ScaleY), FText::FromString(VTChars), GEngine->GetLargeFont(), FLinearColor::White);
				TextItem.EnableShadow(FLinearColor::Black);
				TextItem.Scale = FVector2D(ScaleX, ScaleY);
				TextItem.Draw(Canvas);
			}
			return;
		}
	}
	Super::Draw(Object, X, Y, Width, Height, RenderTarget, Canvas, bAdditionalViewFamily);
}

bool UCPGDTFDescriptionThumbnailRenderer::CanVisualizeAsset(UObject* Object) {
	
	UCPGDTFDescription* Asset = Cast<UCPGDTFDescription>(Object);

	if (Asset != nullptr) {
		if (this->GetTextureFromAsset(Asset) != nullptr) return true;
	}
	return false;
}

UTexture2D* UCPGDTFDescriptionThumbnailRenderer::GetTextureFromAsset(UCPGDTFDescription* Asset) {

	UDMXImportGDTFFixtureType* FixtureType = Cast<UDMXImportGDTFFixtureType>(Asset->FixtureType);

	if (FixtureType == nullptr) return nullptr;
	return Cast<UDMXImportGDTFFixtureType>(Asset->FixtureType)->Thumbnail;
}
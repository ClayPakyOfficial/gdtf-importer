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

#include "Utils/CPGDTFColorWizard.h"
#include "Kismet/KismetMathLibrary.h"

/**
 * Blend the internal color with the given one
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 21 july 2022
 *
 * @param BlendingColor
 * @param Intensity Must be in range [0;1]
 */
void FCPColorWizard::BlendColor(FLinearColor BlendingColor, float Intensity) {
	
	Intensity = FMath::Max(0.0f, FMath::Min(1.0f, Intensity));
	
	// Screen Blending https://en.wikipedia.org/wiki/Blend_modes#Screen
	this->Color.R = 1.0f - (1.0f - this->Color.R)*(1.0f - BlendingColor.R * Intensity);
	this->Color.G = 1.0f - (1.0f - this->Color.G)*(1.0f - BlendingColor.G * Intensity);
	this->Color.B = 1.0f - (1.0f - this->Color.B)*(1.0f - BlendingColor.B * Intensity);


	/* Stolen from QLC+ implementation
	
	// linear blending - https://en.wikipedia.org/w/index.php?title=Alpha_compositing&oldid=1010094191#Alpha_blending
	this->Color.R = BlendingColor.R * Intensity + this->Color.R * (1.0f - Intensity);
	this->Color.G = BlendingColor.G * Intensity + this->Color.G * (1.0f - Intensity);
	this->Color.B = BlendingColor.B * Intensity + this->Color.B * (1.0f - Intensity);

	// non linear blending
	this->Color.R + FMath::Sqrt(FMath::Pow(BlendingColor.R, 2.0f) * Intensity + FMath::Pow(this->Color.R, 2.0f) * (1.0f - Intensity));
	this->Color.G + FMath::Sqrt(FMath::Pow(BlendingColor.G, 2.0f) * Intensity + FMath::Pow(this->Color.G, 2.0f) * (1.0f - Intensity));
	this->Color.B + FMath::Sqrt(FMath::Pow(BlendingColor.B, 2.0f) * Intensity + FMath::Pow(this->Color.B, 2.0f) * (1.0f - Intensity));
	*/
}

void FCPColorWizard::BlendRed(float Intensity) {
	this->BlendColor(FLinearColor(1, 0, 0), Intensity);
}

void FCPColorWizard::BlendGreen(float Intensity) {
	this->BlendColor(FLinearColor(0, 1, 0), Intensity);
}

void FCPColorWizard::BlendBlue(float Intensity) {
	this->BlendColor(FLinearColor(0, 0, 1), Intensity);
}

void FCPColorWizard::BlendWhite(float Intensity) {
	this->BlendColor(FLinearColor(1, 1, 1), Intensity);
}

void FCPColorWizard::BlendCyan(float Intensity) {
	this->BlendColor(FLinearColor(0, 1, 1), Intensity);
}

void FCPColorWizard::BlendMagenta(float Intensity) {
	this->BlendColor(FLinearColor(1, 0, 1), Intensity);
}

void FCPColorWizard::BlendYellow(float Intensity) {
	this->BlendColor(FLinearColor(1, 1, 0), Intensity);
}

void FCPColorWizard::BlendAmber(float Intensity) {
	this->BlendColor(FLinearColor(1, 0.4941176471, 0), Intensity);
}

void FCPColorWizard::BlendLime(float Intensity) {
	this->BlendColor(FLinearColor(0.6784313725, 1, 0.1843137255), Intensity);
}

void FCPColorWizard::BlendBlueGreen(float Intensity) {
	this->BlendColor(FLinearColor(0, 0.5019607843, 0.5019607843), Intensity);
}

void FCPColorWizard::BlendLightBlue(float Intensity) {
	this->BlendColor(FLinearColor(0.6784313725, 0.8470588235, 0.9019607843), Intensity);
}

void FCPColorWizard::BlendPurple(float Intensity) {
	this->BlendColor(FLinearColor(0.5019607843, 0, 0.5019607843), Intensity);
}

void FCPColorWizard::BlendPink(float Intensity) {
	this->BlendColor(FLinearColor(1, 0.7529411765, 0.7960784314), Intensity);
}

void FCPColorWizard::BlendWarmWhite(float Intensity) {
	this->BlendColor(FLinearColor(1, 0.6949019608, 0.4310470588), Intensity);
}

void FCPColorWizard::BlendCoolWhite(float Intensity) {
	this->BlendColor(FLinearColor(0.9514196078, 0.9495568627, 1), Intensity);
}

void FCPColorWizard::BlendUV(float Intensity) {
	this->BlendColor(FLinearColor(0.0392156863, 0, 0.0392156863), Intensity);
}

/**
 * Convert a HSV color representation to a RGB one
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 25 july 2022
 *
 * @param H Hue Must be in range [0; 360]
 * @param S Saturation Must be in range [0; 1]
 * @param V Value Must be in range [0; 1]
 * @return Conversion result
*/
FLinearColor FCPColorWizard::ColorHSVToRGB(float H, float S, float V) {
	return UKismetMathLibrary::HSVToRGBLinear(FLinearColor(H, S, V));
}

/**
 * Convert a RGB color representation to a HSV one
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 25 july 2022
 *
 * @param RGBColor Color to convert
 * @return Conversion result
*/
FVector3f FCPColorWizard::ColorRGBToHSV(FLinearColor RGBColor) {

	FLinearColor HSVColor = UKismetMathLibrary::RGBLinearToHSV(RGBColor);
	return FVector3f(HSVColor.R, HSVColor.G, HSVColor.B);
}

/**
 * Convert a CIE color representation to a RGB one
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 11 july 2022
 *
 * @param InitialColor Color to convert
 * @return Conversion result
*/
FLinearColor FCPColorWizard::ColorCIEToRGB(FDMXColorCIE InitialColor) {

	// This method is stolen from official GDTF builder JS sources (https://fixturebuilder.gdtf-share.com/)

	// Step 1: XYY to XYZ Color space
	float X = 0, Y = 0, Z = 0;

	if (InitialColor.Y != 0) {
		X = (InitialColor.X * InitialColor.YY) / InitialColor.Y;
		Y = InitialColor.YY;
		Z = (1 - InitialColor.X - InitialColor.Y) * (InitialColor.YY / InitialColor.Y);
	}

	X /= 100;
	Y /= 100;
	Z /= 100;

	// Step 2: XYZ to RGB
	FLinearColor RGB;

	RGB.R = (X *  3.2406) + (Y * -1.5372) + (Z * -0.4986);
	RGB.G = (X * -0.9689) + (Y *  1.8758) + (Z *  0.0415);
	RGB.B = (X *  0.0557) + (Y * -0.2040) + (Z *  1.0570);

	// Color correction
	RGB.R = RGB.R > 0.0031308 ? ((1.055 * FMath::Pow(RGB.R, 1.0 / 2.4)) - 0.055) : RGB.R *= 12.92;
	RGB.G = RGB.G > 0.0031308 ? ((1.055 * FMath::Pow(RGB.G, 1.0 / 2.4)) - 0.055) : RGB.G *= 12.92;
	RGB.B = RGB.B > 0.0031308 ? ((1.055 * FMath::Pow(RGB.B, 1.0 / 2.4)) - 0.055) : RGB.B *= 12.92;

	RGB.R = FMath::Min(FMath::Max(0, RGB.R), 1);
	RGB.G = FMath::Min(FMath::Max(0, RGB.G), 1);
	RGB.B = FMath::Min(FMath::Max(0, RGB.B), 1);

	RGB.A = 1.0f;

	return RGB;
}

/**
 * Convert a RGB color representation to a CIE one
 * @author Dorian Gardes - Clay Paky S.R.L.
 * @date 11 july 2022
 *
 * @param RGB Color to convert
 * @return Conversion result
*/
FDMXColorCIE FCPColorWizard::ColorRGBToCIE(FLinearColor RGB) {

	// This method is stolen from official GDTF builder JS sources (https://fixturebuilder.gdtf-share.com/)

	RGB.R = RGB.R > 0.04045 ? (FMath::Pow((RGB.R + 0.055) / 1.055, 2.4)) : RGB.R /= 12.92;
	RGB.G = RGB.G > 0.04045 ? (FMath::Pow((RGB.G + 0.055) / 1.055, 2.4)) : RGB.G /= 12.92;
	RGB.B = RGB.B > 0.04045 ? (FMath::Pow((RGB.B + 0.055) / 1.055, 2.4)) : RGB.B /= 12.92;

	RGB.R *= 100;
	RGB.G *= 100;
	RGB.B *= 100;

	float X = (RGB.R * 0.4124) + (RGB.G * 0.3576) + (RGB.B * 0.1805);
	float Y = (RGB.R * 0.2126) + (RGB.G * 0.7152) + (RGB.B * 0.0722);
	float Z = (RGB.R * 0.0193) + (RGB.G * 0.1192) + (RGB.B * 0.9505);

	FDMXColorCIE CIE;
	CIE.X = (X + Y + Z) != 0 ? X / (X + Y + Z) : 0.3127;
	CIE.Y = (X + Y + Z) != 0 ? X / (X + Y + Z) : 0.3290;
	CIE.YY = Y;

	return CIE;
}
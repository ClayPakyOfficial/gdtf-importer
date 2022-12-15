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
 * Wizard to simplify the color mixing <br>
 * **WARNING:** This doesn't blend alpha channel.
 */
class CLAYPAKYGDTFIMPORTER_API FCPColorWizard {

protected:

	FLinearColor Color;

public:

	/// Constructor. Set the default color to Black with no transparency.
	FCPColorWizard() { this->Color = FLinearColor(0, 0, 0, 1); }

	FLinearColor GetColor() { return this->Color; }
	void SetColor(FLinearColor NewColor) { this->Color = NewColor; }
	
	/**
	 * @param Red   Must be in range [0;1]
	 * @param Green Must be in range [0;1]
	 * @param Blue  Must be in range [0;1]
	 */
	void SetRGB(float Red, float Green, float Blue) { this->SetColor(FLinearColor(Red, Green, Blue, 1)); }

	/**
	 * Add Red to the color
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 21 july 2022
	 * 
	 * @param Intensity Must be in range [0;1]
	 */
	void BlendRed(float Intensity);

	/**
	 * Add Green to the color
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 21 july 2022
	 *
	 * @param Intensity Must be in range [0;1]
	 */
	void BlendGreen(float Intensity);

	/**
	 * Add Blue to the color
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 21 july 2022
	 *
	 * @param Intensity Must be in range [0;1]
	 */
	void BlendBlue(float Intensity);

	/**
	 * Add White to the color
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 21 july 2022
	 *
	 * @param Intensity Must be in range [0;1]
	 */
	void BlendWhite(float Intensity);

	/**
	 * Add Cyan to the color
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 21 july 2022
	 *
	 * @param Intensity Must be in range [0;1]
	 */
	void BlendCyan(float Intensity);

	/**
	 * Add Magenta to the color
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 21 july 2022
	 *
	 * @param Intensity Must be in range [0;1]
	 */
	void BlendMagenta(float Intensity);

	/**
	 * Add Yellow to the color
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 21 july 2022
	 *
	 * @param Intensity Must be in range [0;1]
	 */
	void BlendYellow(float Intensity);

	/**
	 * Add Amber to the color
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 21 july 2022
	 *
	 * @param Intensity Must be in range [0;1]
	 */
	void BlendAmber(float Intensity);

	/**
	 * Add Lime to the color
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 21 july 2022
	 *
	 * @param Intensity Must be in range [0;1]
	 */
	void BlendLime(float Intensity);

	/**
	 * Add BlueGreen to the color
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 21 july 2022
	 *
	 * @param Intensity Must be in range [0;1]
	 */
	void BlendBlueGreen(float Intensity);

	/**
	 * Add LightBlue to the color
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 21 july 2022
	 *
	 * @param Intensity Must be in range [0;1]
	 */
	void BlendLightBlue(float Intensity);

	/**
	 * Add Purple to the color
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 21 july 2022
	 *
	 * @param Intensity Must be in range [0;1]
	 */
	void BlendPurple(float Intensity);

	/**
	 * Add Pink to the color
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 21 july 2022
	 *
	 * @param Intensity Must be in range [0;1]
	 */
	void BlendPink(float Intensity);

	/**
	 * Add WarmWhite to the color (3000K)
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 21 july 2022
	 *
	 * @param Intensity Must be in range [0;1]
	 */
	void BlendWarmWhite(float Intensity);

	/**
	 * Add CoolWhite to the color (7000K)
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 21 july 2022
	 *
	 * @param Intensity Must be in range [0;1]
	 */
	void BlendCoolWhite(float Intensity);

	/**
	 * Add UV to the color
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 21 july 2022
	 *
	 * @param Intensity Must be in range [0;1]
	 */
	void BlendUV(float Intensity);

	/**
	 * Blend the internal color with the given one
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 21 july 2022
	 *
	 * @param BlendingColor
	 * @param Intensity Must be in range [0;1]
	 */
	void BlendColor(FLinearColor BlendingColor, float Intensity);

	/**
	 * Convert a HSV color representation to a RGB one
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 25 july 2022
	 *
	 * @param H Hue Must be in range [0; 360]
	 * @param S Saturation Must be in range [0; 1]
	 * @param V Value Must be in range [0; 1]
	 * @return Conversion result
	 */
	static FLinearColor ColorHSVToRGB(float H, float S, float V);

	/**
	 * Convert a RGB color representation to a HSV one
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 25 july 2022
	 *
	 * @param RGBColor Color to convert
	 * @return Conversion result
	*/
	static FVector3f ColorRGBToHSV(FLinearColor RGBColor);

	/**
	 * Convert a CIE color representation to a RGB one
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 11 july 2022
	 *
	 * @param InitialColor Color to convert
	 * @return Conversion result
	*/
	static FLinearColor ColorCIEToRGB(FDMXColorCIE InitialColor);

	/**
	 * Convert a RGB color representation to a CIE one
	 * @author Dorian Gardes - Clay Paky S.P.A.
	 * @date 11 july 2022
	 *
	 * @param RGB Color to convert
	 * @return Conversion result
	 */
	static FDMXColorCIE ColorRGBToCIE(FLinearColor RGB);
};
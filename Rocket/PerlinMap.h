#pragma once

class PerlinMap
{
public:
										PerlinMap();
	const int&							GetPermutationDescriptorIdx() const;
	const int&							GetGradientsDescriptorIdx() const;

protected:
	int									mPermutationIdx;
	int									mGradientsIdx;
	int									mPermutationDescriptorIdx;
	int									mGradientsDescriptorIdx;
	int									mPermutationUploadIdx;
	int									mGradientsUploadIdx;
};

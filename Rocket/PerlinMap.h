#pragma once

class PerlinMap
{
public:
										PerlinMap();

	int									mPermutationIdx;
	int									mGradientsIdx;
	int									mPermutationDescriptorIdx;
	int									mGradientsDescriptorIdx;
	int									mPermutationUploadIdx;
	int									mGradientsUploadIdx;
};

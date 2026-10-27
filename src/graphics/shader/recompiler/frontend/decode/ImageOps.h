#ifndef EMULATOR_INCLUDE_EMULATOR_GRAPHICS_SHADER_RECOMPILER_IMAGEOPS_H_
#define EMULATOR_INCLUDE_EMULATOR_GRAPHICS_SHADER_RECOMPILER_IMAGEOPS_H_

#include <algorithm>
#include <bit>

#include "graphics/shader/recompiler/frontend/decode/ShaderDecoder.h"

namespace Libs::Graphics::ShaderRecompiler::Decoder {

void DecodeMimg(uint32_t pc, std::span<const uint32_t> code, uint32_t word_index,
                Instruction& inst);

const char* MimgSampledOpcodeName(uint32_t opcode);

struct ImageAddressComponent {
	uint32_t bit_offset;
	uint32_t bit_width;
};

inline ImageAddressComponent ImageAddressComponentLayout(uint32_t flags, ImageDimension dimension,
                                                         uint32_t component) {
	if ((flags & ImageSampleFlagA16) == 0u) return {component * 32u, 32u};
	// A16 offsets, bias and PCF reference each take a whole VGPR, the bias as a half in its low
	// 16 bits. The other components pack two per VGPR, except that each 1D derivative and the last
	// two 3D ones take the low half of their own VGPR.
	const bool     has_bias    = (flags & ImageSampleFlagBias) != 0u;
	const uint32_t bias_index  = (flags & ImageSampleFlagOffset) != 0u ? 1u : 0u;
	const uint32_t whole_vgprs = std::popcount(
	    flags & (ImageSampleFlagOffset | ImageSampleFlagBias | ImageSampleFlagCompare));
	if (component < whole_vgprs) {
		return {component * 32u, has_bias && component == bias_index ? 16u : 32u};
	}
	const auto gradient_components = [](ImageDimension dim) {
		switch (dim) {
			case ImageDimension::Dim1D:
			case ImageDimension::Dim1DArray: return 1u;
			case ImageDimension::Dim3D: return 3u;
			default: return 2u;
		}
	};
	const uint32_t axes = (flags & ImageSampleFlagDerivative) != 0u
	                          ? gradient_components(dimension)
	                          : 0u;
	const uint32_t unpaired = axes % 2u * 2u;
	const uint32_t paired   = axes * 2u - unpaired;
	const uint32_t index    = component - whole_vgprs;
	const uint32_t half     = index + std::clamp(index, paired, paired + unpaired) - paired;
	return {whole_vgprs * 32u + half * 16u, 16u};
}
uint32_t ImageAddressDwordCount(uint32_t flags, ImageDimension dimension, uint32_t components);

} // namespace Libs::Graphics::ShaderRecompiler::Decoder

#endif /* EMULATOR_INCLUDE_EMULATOR_GRAPHICS_SHADER_RECOMPILER_IMAGEOPS_H_ */

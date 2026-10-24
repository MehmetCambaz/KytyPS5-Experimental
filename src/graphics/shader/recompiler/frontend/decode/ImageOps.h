#ifndef EMULATOR_INCLUDE_EMULATOR_GRAPHICS_SHADER_RECOMPILER_IMAGEOPS_H_
#define EMULATOR_INCLUDE_EMULATOR_GRAPHICS_SHADER_RECOMPILER_IMAGEOPS_H_

#include "graphics/shader/recompiler/frontend/decode/ShaderDecoder.h"

namespace Libs::Graphics::ShaderRecompiler::Decoder {

void DecodeMimg(uint32_t pc, std::span<const uint32_t> code, uint32_t word_index,
                Instruction& inst);

const char* MimgSampleOpcodeName(uint32_t opcode);

struct ImageAddressComponent {
	uint32_t bit_offset;
	uint32_t bit_width;
};

inline ImageAddressComponent ImageAddressComponentLayout(uint32_t flags, uint32_t component) {
	const auto width = [flags](uint32_t index) {
		if ((flags & ImageSampleFlagA16) == 0u) return 32u;
		uint32_t cursor = 0;
		if ((flags & ImageSampleFlagOffset) != 0u) {
			if (index == cursor++) return 32u;
		}
		if ((flags & ImageSampleFlagBias) != 0u) {
			if (index == cursor++) return 16u;
		}
		if ((flags & ImageSampleFlagCompare) != 0u && index == cursor) return 32u;
		return 16u;
	};
	uint32_t offset = 0;
	for (uint32_t index = 0; index < component; index++) {
		const auto bits = width(index);
		if (bits == 32u) offset = (offset + 31u) & ~31u;
		offset += bits;
	}
	const auto bits = width(component);
	if (bits == 32u) offset = (offset + 31u) & ~31u;
	return {offset, bits};
}
uint32_t ImageAddressDwordCount(uint32_t flags, uint32_t components);

} // namespace Libs::Graphics::ShaderRecompiler::Decoder

#endif /* EMULATOR_INCLUDE_EMULATOR_GRAPHICS_SHADER_RECOMPILER_IMAGEOPS_H_ */

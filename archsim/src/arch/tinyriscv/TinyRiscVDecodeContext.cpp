/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

#include "gensim/gensim_decode_context.h"
#include "arch/tinyriscv/TinyRiscVDecodeContext.h"
#include "util/ComponentManager.h"
#include "core/thread/ThreadInstance.h"

using namespace archsim::arch::tinyriscv;

uint32_t TinyRiscVDecodeContext::DecodeSync(archsim::MemoryInterface &interface, Address address, uint32_t mode, gensim::BaseDecode *&target)
{
	target = arch_.GetISA(mode).GetNewDecode();

	return arch_.GetISA(mode).DecodeInstr(address, &interface, *target);
}

class TinyRiscVDecodeTranslationContext : public gensim::DecodeTranslateContext
{
	void Translate(archsim::core::thread::ThreadInstance *cpu, const gensim::BaseDecode &insn, gensim::DecodeContext &decode, captive::shared::IRBuilder &builder) override
	{
		// nothing necessary here
	}

};

RegisterComponent(gensim::DecodeTranslateContext, TinyRiscVDecodeTranslationContext, "tinyriscv", "TinyRISCV");

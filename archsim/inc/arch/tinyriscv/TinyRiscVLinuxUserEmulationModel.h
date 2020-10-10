/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

#ifndef TINYRISCVLINUXUSEREMULATIONMODEL_H
#define	TINYRISCVLINUXUSEREMULATIONMODEL_H

#include "abi/LinuxUserEmulationModel.h"

namespace archsim
{
	namespace arch
	{
		namespace tinyriscv
		{
			class TinyRiscVLinuxUserEmulationModel : public archsim::abi::LinuxUserEmulationModel
			{
			public:
				TinyRiscVLinuxUserEmulationModel();
				~TinyRiscVLinuxUserEmulationModel();

				bool Initialise(System& system, uarch::uArch& uarch) override;
				void Destroy() override;

				archsim::abi::ExceptionAction HandleException(archsim::core::thread::ThreadInstance *cpu, uint64_t category, uint64_t data);
				bool InvokeSignal(int signum, uint32_t next_pc, archsim::abi::SignalData* data);

				bool PrepareBoot(System& system);

				gensim::DecodeContext* GetNewDecodeContext(archsim::core::thread::ThreadInstance& cpu) override;


			private:

			};
		}
	}
}

#endif	// TINYRISCVLINUXUSEREMULATIONMODEL_H


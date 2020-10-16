/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

/*
 * arch/arm/ArmLinuxUserEmulationModel.cpp
 */
#include "arch/tinyriscv/TinyRiscVLinuxUserEmulationModel.h"
#include "arch/tinyriscv/TinyRiscVDecodeContext.h"

#include "util/ComponentManager.h"
#include "util/LogContext.h"
#include "util/SimOptions.h"
#include "system.h"

#ifdef CONFIG_GFX
#ifdef CONFIG_SDL
#include "abi/devices/gfx/SDLScreen.h"
#endif
#include "abi/devices/generic/DoomDevice.h"
#endif

using namespace archsim::abi;
using namespace archsim::abi::memory;
using namespace archsim::arch::tinyriscv;

UseLogContext(LogEmulationModelUser);
DeclareChildLogContext(LogEmulationModelTinyRiscVLinux, LogEmulationModelUser, "TinyRISCV-Linux");

class TRVLinuxUserEmulationModel : public TinyRiscVLinuxUserEmulationModel
{
public:
	TRVLinuxUserEmulationModel() : TinyRiscVLinuxUserEmulationModel() {}
};


RegisterComponent(archsim::abi::EmulationModel, TRVLinuxUserEmulationModel, "tinyriscv-user", "TinyRISCV Linux user emulation model");

TinyRiscVLinuxUserEmulationModel::TinyRiscVLinuxUserEmulationModel() : LinuxUserEmulationModel("tinyriscv", true, AuxVectorEntries("tinyriscv", 0, 0)) { }

TinyRiscVLinuxUserEmulationModel::~TinyRiscVLinuxUserEmulationModel() { }

bool TinyRiscVLinuxUserEmulationModel::Initialise(System& system, uarch::uArch& uarch)
{
	if (!LinuxUserEmulationModel::Initialise(system, uarch))
		return false;

	return true;
}

void TinyRiscVLinuxUserEmulationModel::Destroy()
{
	LinuxUserEmulationModel::Destroy();
}

bool TinyRiscVLinuxUserEmulationModel::PrepareBoot(System& system)
{
	if (!LinuxUserEmulationModel::PrepareBoot(system))
		return false;

	LC_DEBUG1(LogEmulationModelTinyRiscVLinux) << "Initialising TinyRISCV Kernel Helpers";

	memory::guest_addr_t kernel_helper_region = 0xffff0000_ga;
	GetMemoryModel().GetMappingManager()->MapRegion(kernel_helper_region, 0x4000, (memory::RegionFlags)(memory::RegFlagRead | memory::RegFlagWrite), "[eabi]");

	/* random data */
	GetMemoryModel().Write32(0xffff0000_ga, 0xbabecafe);
	GetMemoryModel().Write32(0xffff0004_ga, 0xdeadbabe);
	GetMemoryModel().Write32(0xffff0008_ga, 0xfeedc0de);
	GetMemoryModel().Write32(0xffff000c_ga, 0xcafedead);

	GetMainThread()->GetArch().GetISA("tinyriscv").GetBehaviours().GetBehaviour("tinyriscv_set_machmode").Invoke(GetMainThread(), {3});

	return true;
}

gensim::DecodeContext* TinyRiscVLinuxUserEmulationModel::GetNewDecodeContext(archsim::core::thread::ThreadInstance& cpu)
{
	return new arch::tinyriscv::TinyRiscVDecodeContext(cpu.GetArch());
}

bool TinyRiscVLinuxUserEmulationModel::InvokeSignal(int signum, uint32_t next_pc, SignalData* data)
{
	assert(false);

	//TODO

	return false;
}

archsim::abi::ExceptionAction TinyRiscVLinuxUserEmulationModel::HandleException(archsim::core::thread::ThreadInstance *cpu, uint64_t category, uint64_t data)
{
	if(category == 1024) {
		GetSystem().GetPubSub().Publish(PubSubType::L1ICacheFlush, (void*)(uint64_t)0);
		return archsim::abi::ResumeNext;
	}

	if(category == 11) {
		archsim::abi::SyscallResponse response;
		response.action = ResumeNext;

		archsim::abi::SyscallRequest request {0, cpu, 0, 0, 0, 0, 0, 0};


        uint32_t* registers = (uint32_t*)cpu->GetRegisterFile();

        request.syscall = registers[17];

        request.arg0 = registers[10];
        request.arg1 = registers[11];
        request.arg2 = registers[12];
        request.arg3 = registers[13];
        request.arg4 = registers[14];
        request.arg5 = registers[15];


		if(EmulateSyscall(request, response)) {

		} else {
			LC_ERROR(LogEmulationModelTinyRiscVLinux) << "Syscall not supported: " << std::hex << "0x" << request.syscall << "(" << std::dec << request.syscall << ")";
			response.result = -1;
		}

		registers = (uint32_t*)cpu->GetRegisterFile();
		registers[10] = response.result;

		// xxx arm hax
		// currently a syscall cannot return an action other than resume, so
		// we need to exit manually here.
		if(request.syscall == 93) {
			cpu->SendMessage(archsim::core::thread::ThreadMessage::Halt);
			return AbortSimulation;
		}

		// increment PC
		cpu->SetPC(cpu->GetPC()+4);

		return response.action;
	}

	return AbortSimulation;
}

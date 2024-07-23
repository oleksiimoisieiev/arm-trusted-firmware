/*
 * Copyright 2024 EPAM Systems
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <drivers/scmi-msg.h>

#include <rpi5_smc.h>

/* Setup Standard Services */
static int32_t rpi5_svc_setup(void)
{
	return 0;
}

/*
 * Top-level Standard Service SMC handler.
 */
static uintptr_t rpi5_svc_smc_handler(uint32_t smc_fid, u_register_t x1,
					  u_register_t x2, u_register_t x3,
					  u_register_t x4, void *cookie,
					  void *handle, u_register_t flags)
{
	uint32_t ret1 = 0U;

	switch (smc_fid) {
	case RPI5_SIP_SMC_SCMI:
		scmi_smt_fastcall_smc_entry(0);
		break;

	default:
		WARN("Unknown RPI5 Service Call: 0x%x\n", smc_fid);
		ret1 = SMC_UNK;
		break;
	}

	SMC_RET1(handle, ret1);
}

/* Register Standard Service Calls as runtime service */
DECLARE_RT_SVC(rpi5_sip_svc,
	       OEN_SIP_START,
	       OEN_SIP_END,
	       SMC_TYPE_FAST,
	       rpi5_svc_setup,
	       rpi5_svc_smc_handler
);

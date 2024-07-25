/*
 * Copyright 2024 EPAM Systems
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <drivers/scmi.h>
#include <lib/utils_def.h>
#include <platform_def.h>

size_t plat_scmi_rstd_count(unsigned int agent_id __unused)
{
	return 0U;
}

const char *plat_scmi_rstd_get_name(unsigned int agent_id __unused,
				  unsigned int scmi_id __unused)
{
	return NULL;
}

int32_t plat_scmi_rstd_autonomous(unsigned int agent_id __unused,
				unsigned int scmi_id __unused,
				unsigned int state __unused)
{
	return SCMI_NOT_SUPPORTED;
}

int32_t plat_scmi_rstd_set_state(unsigned int agent_id __unused,
			       unsigned int scmi_id __unused,
			       bool assert_not_deassert __unused)
{
	return SCMI_NOT_SUPPORTED;
}

/*
 * Copyright (c) 2024, Mario Bălănică <mariobalanica02@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <rpi_shared.h>
#include <common/debug.h>

extern void rpi5_init_scmi_server(void);

void plat_rpi_bl31_custom_setup(void)
{
	INFO("!!! %s \n", __func__);
	rpi5_init_scmi_server();
}

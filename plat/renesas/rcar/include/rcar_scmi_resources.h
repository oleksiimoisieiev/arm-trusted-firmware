/*
 * Copyright (c) 2021 EPAM Systems Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef RCAR_SCMI_RESOURCES_H
#define RCAR_SCMI_RESOURCES_H

enum rcar_scmi_devid {
	RCAR_SCMIDEV_EAVB,	/*  0 */
	RCAR_SCMIDEV_HDMI0,	/*  1 */
	RCAR_SCMIDEV_HDMI1,	/*  2 */
	RCAR_SCMIDEV_I2C0,	/*  3 */
	RCAR_SCMIDEV_I2C1,	/*  4 */
	RCAR_SCMIDEV_I2C2,	/*  5 */
	RCAR_SCMIDEV_I2C3,	/*  6 */
	RCAR_SCMIDEV_I2C4,	/*  7 */
	RCAR_SCMIDEV_XHCI0,	/*  8 */
	RCAR_SCMIDEV_OHCI0,	/*  9 */
	RCAR_SCMIDEV_EHCI0,	/* 10 */
	RCAR_SCMIDEV_OHCI1,	/* 11 */
	RCAR_SCMIDEV_EHCI1,	/* 12 */
	RCAR_SCMIDEV_USB_DMAC0,	/* 13 */
	RCAR_SCMIDEV_USB_DMAC1,	/* 14 */
	RCAR_SCMIDEV_USB_DMAC2,	/* 15 */
	RCAR_SCMIDEV_USB_DMAC3,	/* 16 */
	RCAR_SCMIDEV_USB2_PHY0,	/* 17 */
	RCAR_SCMIDEV_USB2_PHY1,	/* 18 */
	RCAR_SCMIDEV_HSUSB,	/* 19 */
	RCAR_SCMIDEV_SOUND,	/* 20 */
	RCAR_SCMIDEV_AUDMA0,	/* 21 */
	RCAR_SCMIDEV_AUDMA1,	/* 22 */
	RCAR_SCMIDEV_MAX
};

enum rcar_scmi_rst_offset {
	RCAR_SCMIRST_EAVB,	/*  0*/
	RCAR_SCMIRST_HDMI0,	/*  1 */
	RCAR_SCMIRST_HDMI1,	/*  2 */
	RCAR_SCMIRST_I2C0,	/*  3 */
	RCAR_SCMIRST_I2C1,	/*  4 */
	RCAR_SCMIRST_I2C2,	/*  5 */
	RCAR_SCMIRST_I2C3,	/*  6 */
	RCAR_SCMIRST_I2C4,	/*  7 */
	RCAR_SCMIRST_XHCI0,	/*  8 */
	RCAR_SCMIRST_USB2_01,	/*  9 */
	RCAR_SCMIRST_USB2_02,	/* 10 */
	RCAR_SCMIRST_USB2_1,	/* 11 */
	RCAR_SCMIRST_USB_DMAC0,	/* 12 */
	RCAR_SCMIRST_USB_DMAC1,	/* 13 */
	RCAR_SCMIRST_USB_DMAC2,	/* 14 */
	RCAR_SCMIRST_USB_DMAC3,	/* 15 */
	RCAR_SCMIRST_SSI,	/* 16 */
	RCAR_SCMIRST_SSI9,	/* 17 */
	RCAR_SCMIRST_SSI8,	/* 18 */
	RCAR_SCMIRST_SSI7,	/* 19 */
	RCAR_SCMIRST_SSI6,	/* 20 */
	RCAR_SCMIRST_SSI5,	/* 21 */
	RCAR_SCMIRST_SSI4,	/* 22 */
	RCAR_SCMIRST_SSI3,	/* 23 */
	RCAR_SCMIRST_SSI2,	/* 24 */
	RCAR_SCMIRST_SSI1,	/* 25 */
	RCAR_SCMIRST_SSI0,	/* 26 */
	RCAR_SCMIRST_AUDMAC1,	/* 27 */
	RCAR_SCMIRST_AUDMAC0,	/* 28 */
	RCAR_SCMIRST_MAX
};

enum rcar_scmi_clk {
	RCAR_SCMICLK_EAVB,	/*  0 */
	RCAR_SCMICLK_XHCI0,	/*  1 */
	RCAR_SCMICLK_EHCI0,	/*  2 */
	RCAR_SCMICLK_HSUSB,	/*  3 */
	RCAR_SCMICLK_EHCI1,	/*  4 */
	RCAR_SCMICLK_USB_DMAC0,	/*  5 */
	RCAR_SCMICLK_USB_DMAC1,	/*  6 */
	RCAR_SCMICLK_USB_DMAC30,/*  7 */
	RCAR_SCMICLK_USB_DMAC31,/*  8 */
	RCAR_SCMICLK_SSI_ALL,	/*  9 */
	RCAR_SCMICLK_SSI9,	/* 10 */
	RCAR_SCMICLK_SSI8,	/* 11 */
	RCAR_SCMICLK_SSI7,	/* 12 */
	RCAR_SCMICLK_SSI6,	/* 13 */
	RCAR_SCMICLK_SSI5,	/* 14 */
	RCAR_SCMICLK_SSI4,	/* 15 */
	RCAR_SCMICLK_SSI3,	/* 16 */
	RCAR_SCMICLK_SSI2,	/* 17 */
	RCAR_SCMICLK_SSI1,	/* 18 */
	RCAR_SCMICLK_SSI0,	/* 19 */
	RCAR_SCMICLK_SCU_ALL,	/* 20 */
	RCAR_SCMICLK_SCU_DVC1,	/* 21 */
	RCAR_SCMICLK_SCU_DVC0,	/* 22 */
	RCAR_SCMICLK_SCU_MIX1,	/* 23 */
	RCAR_SCMICLK_SCU_MIX0,	/* 24 */
	RCAR_SCMICLK_SCU_SRC9,	/* 25 */
	RCAR_SCMICLK_SCU_SRC8,	/* 26 */
	RCAR_SCMICLK_SCU_SRC7,	/* 27 */
	RCAR_SCMICLK_SCU_SRC6,	/* 28 */
	RCAR_SCMICLK_SCU_SRC5,	/* 29 */
	RCAR_SCMICLK_SCU_SRC4,	/* 30 */
	RCAR_SCMICLK_SCU_SRC3,	/* 31 */
	RCAR_SCMICLK_SCU_SRC2,	/* 32 */
	RCAR_SCMICLK_SCU_SRC1,	/* 33 */
	RCAR_SCMICLK_SCU_SRC0,	/* 34 */
	RCAR_SCMICLK_AUDMAC1,	/* 35 */
	RCAR_SCMICLK_AUDMAC0,	/* 36 */
	RCAR_SCMICLK_HDMI,
	RCAR_SCMICLK_HDMI0,
	RCAR_SCMICLK_HDMI1,
	RCAR_CLK_EXTAL,
	RCAR_CLK_MAIN,
	RCAR_CLK_PLL1,
	RCAR_CLK_PLL1D2,
	RCAR_CLK_S0,
	RCAR_CLK_S1,
	RCAR_CLK_S3,
	RCAR_CLK_S0D6,
	RCAR_CLK_S1D2,
	RCAR_CLK_S3D1,
	RCAR_CLK_S3D2,
	RCAR_CLK_S3D4,
	RCAR_CLK_MAX,
	RCAR_SCMICLK_MAX = RCAR_SCMICLK_HDMI /* end of SCMI exported clocks */
};

extern const struct scmi_device rcar_devices[RCAR_SCMIDEV_MAX];
extern struct scmi_clk rcar_clocks[RCAR_CLK_MAX];
extern struct scmi_reset rcar_resets[RCAR_SCMIRST_MAX];

#endif /* RCAR_SCMI_RESOURCES_H */

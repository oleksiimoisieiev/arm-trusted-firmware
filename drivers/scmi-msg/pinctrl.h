/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2023, EPAM Systems
 */
#ifndef SCMI_MSG_PINCTRL_DOMAIN_H
#define SCMI_MSG_PINCTRL_DOMAIN_H

#include <stdbool.h>
#include <stdint.h>

#include <lib/utils_def.h>

#define SCMI_PROTOCOL_VERSION_PINCTRL_DOMAIN	0x10000U

/*
 * Identifiers of the SCMI Pinctrl Domain Management Protocol commands
 */
enum scmi_pinctrl_domain_command_id {
	SCMI_PINCTRL_DOMAIN_ATTRIBUTES = 0x03,
	SCMI_PINCTRL_DOMAIN_LIST_ASSOCIATIONS = 0x04,
	SCMI_PINCTRL_DOMAIN_CONFIG_GET = 0x05,
	SCMI_PINCTRL_DOMAIN_CONFIG_SET = 0x6,
	SCMI_PINCTRL_DOMAIN_FUNCTION_SELECT = 0x7,
	SCMI_PINCTRL_DOMAIN_REQUEST = 0x8,
	SCMI_PINCTRL_DOMAIN_RELEASE = 0x9,
	SCMI_PINCTRL_DOMAIN_NAME_GET = 0xa,
	SCMI_PINCTRL_DOMAIN_SET_PERMISSIONS = 0xb,
};

/*
 * Identifiers of the SCMI Pinctrl Domain Management Protocol responses
 */
enum scmi_pinctrl_domain_response_id {
	SCMI_PINCTRL_ISSUED = 0x00,
	SCMI_PINCTRL_COMPLETE = 0x04,
};

/*
 * PROTOCOL_ATTRIBUTES
 */

#define SCMI_PINCTRL_DOMAIN_COUNT_MASK		GENMASK_32(15, 0)

struct scmi_reset_domain_protocol_attributes_p2a {
	int32_t status;
	uint32_t attributes;
};

/* Value for scmi_reset_domain_attributes_p2a:flags */
#define SCMI_PINCTRL_DOMAIN_ATTR_ASYNC		BIT(31)
#define SCMI_PINCTRL_DOMAIN_ATTR_NOTIF		BIT(30)

/* Value for scmi_reset_domain_attributes_p2a:latency */
#define SCMI_PINCTRL_DOMAIN_ATTR_UNK_LAT		0x7fffffffU
#define SCMI_PINCTRL_DOMAIN_ATTR_MAX_LAT		0x7ffffffeU

/* Macro for scmi_reset_domain_attributes_p2a:name */
#define SCMI_PINCTRL_DOMAIN_ATTR_NAME_SZ		16U

struct scmi_reset_domain_attributes_a2p {
	uint32_t domain_id;
};

struct scmi_reset_domain_attributes_p2a {
	int32_t status;
	uint32_t flags;
	uint32_t latency;
	char name[SCMI_PINCTRL_DOMAIN_ATTR_NAME_SZ];
};

/*
 * RESET
 */

/* Values for scmi_reset_domain_request_a2p:flags */
#define SCMI_PINCTRL_DOMAIN_ASYNC			BIT(2)
#define SCMI_PINCTRL_DOMAIN_EXPLICIT		BIT(1)
#define SCMI_PINCTRL_DOMAIN_AUTO			BIT(0)

struct scmi_reset_domain_request_a2p {
	uint32_t domain_id;
	uint32_t flags;
	uint32_t reset_state;
};

struct scmi_reset_domain_request_p2a {
	int32_t status;
};

/*
 * RESET_NOTIFY
 */

/* Values for scmi_reset_notify_p2a:flags */
#define SCMI_PINCTRL_DOMAIN_DO_NOTIFY		BIT(0)

struct scmi_reset_domain_notify_a2p {
	uint32_t domain_id;
	uint32_t notify_enable;
};

struct scmi_reset_domain_notify_p2a {
	int32_t status;
};

/*
 * RESET_COMPLETE
 */

struct scmi_reset_domain_complete_p2a {
	int32_t status;
	uint32_t domain_id;
};

/*
 * RESET_ISSUED
 */

struct scmi_reset_domain_issued_p2a {
	uint32_t domain_id;
	uint32_t reset_state;
};

#endif /* SCMI_MSG_PINCTRL_DOMAIN_H */

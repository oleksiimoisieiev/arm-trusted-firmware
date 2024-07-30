// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2024, EPAM Systems
 */
#include <cdefs.h>
#include <string.h>

#include <drivers/scmi-msg.h>
#include <drivers/scmi.h>
#include <lib/utils.h>
#include <lib/utils_def.h>

#include "common.h"
#define PP(fmt, ...) \
	INFO("==== %s %d " fmt "\n", __func__, __LINE__, ##__VA_ARGS__);

#pragma weak plat_scmi_pinctrl_count
#pragma weak plat_scmi_pinctrl_get_name
#pragma weak plat_scmi_pinctrl_autonomous
#pragma weak plat_scmi_pinctrl_set_state

//TODO move to header
struct pinctrl_pin {
	uint16_t pin;
	uint16_t enum_id;
	const char *name;
	unsigned int configs;
};

size_t plat_scmi_pinctrl_count(unsigned int agent_id __unused)
{
	return 0U;
}

uint16_t pinctrl_get_groups_count()
{
	return 2U;
}

uint16_t pinctrl_get_functions_count()
{
	return 2U;
}

//TODO remove that
//allocate pins
static struct pinctrl_pin l_pins[] = {
		{
			.configs = 1,
			.enum_id = 1,
			.name = "test_pin",
			.pin = 1
		},
		{
			.configs = 1,
			.enum_id = 2,
			.name = "test_pin2",
			.pin = 2
		}
};

int pinctrl_get_pins(const struct pinctrl_pin **pins, unsigned *num_pins)
{
	*pins = &l_pins[0];
	*num_pins = 2;
	return 0U;
}

const char *pinctrl_get_function_name(unsigned selector)
{
  return "function";
}

const char *pinctrl_get_group_name(unsigned selector) {
	return "group";
}

const char *plat_scmi_pinctrl_get_name(unsigned int agent_id __unused,
				  unsigned int scmi_id __unused)
{
	return NULL;
}
static const unsigned l_pins_g[] = {1, 2};

int pinctrl_get_group_pins(unsigned selector, const unsigned **pins,
					 unsigned *num_pins)
{
	*pins = (const unsigned *)&l_pins_g[0];
	*num_pins = 2;
	return 0;
}

static const int l_groups[] ={1, 2};
int pinctrl_get_function_groups(unsigned selector, const int **groups,
			  unsigned *num_groups)
{
	*groups = (const int *)&l_groups[0];
	*num_groups = 2;
	return 0;
}

int pinctrl_request(unsigned offset)
{
	return 0;
}
int pinctrl_free(unsigned offset)
{
	return 0;
}

static void report_version(struct scmi_msg *msg)
{
	struct scmi_protocol_version_p2a return_values = {
		.status = SCMI_SUCCESS,
		.version = SCMI_PROTOCOL_VERSION_PINCTRL,
	};

	PP("scmi: Process pinctrl protocol_version\n");

	if (msg->in_size != 0U) {
		PP("");
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	scmi_write_response(msg, &return_values, sizeof(return_values));
}

static void report_attributes(struct scmi_msg *msg)
{
	struct attrs_rx {
		int32_t status;
		uint32_t attrs_low;
		uint32_t attrs_high;
	} return_values = {
			.status = SCMI_SUCCESS
	};

	const struct pinctrl_pin *pins;
	unsigned nr_pins;
	int ret;

	uint16_t nr_groups = pinctrl_get_groups_count();
	uint16_t nr_functions = pinctrl_get_functions_count();

//	struct scmi_protocol_attributes_p2a return_values = {
//		.status = SCMI_SUCCESS,
//		.attributes = plat_scmi_pinctrl_count(msg->agent_id),
//	};

	if (msg->in_size != 0U) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}
	ret = pinctrl_get_pins(&pins, &nr_pins);
	if (ret) {
		ERROR("scmi: pinctrl_get_pins failed with ret = %d\n", ret);
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	};

	return_values.attrs_low = FLD(GENMASK(31, 16), nr_groups) |
		FLD(GENMASK(15,  0), nr_pins);
	return_values.attrs_high = FLD(GENMASK(15, 0), nr_functions);

	WARN("scmi: Process pinctrl protocol_attrs group_max = %d, func_max = %d nr_pins = %d\n",
		    nr_groups, nr_functions, nr_pins);

	scmi_write_response(msg, &return_values, sizeof(return_values));
}

static void report_message_attributes(struct scmi_msg *msg)
{
	struct attrs_rx_m {
		int32_t status;
		uint32_t attrs;
	} return_values = {
			.status = SCMI_SUCCESS,
			.attrs = 0,
	};

	if (msg->in_size != sizeof(id)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	scmi_write_response(msg, &return_values, sizeof(return_values));
}

struct pin_attrs_rx {
	int32_t status;
	uint32_t attributes;
	char name[16];
};

static int set_rx(struct pin_attrs_rx *rx, const char *name, unsigned n_elems)
{
	assert(name != NULL);

	rx->attributes = FLD(GENMASK(15, 0), n_elems);
	if (sizeof(name) > sizeof(rx->name))
		rx->attributes |= FLD(31, 1);
	else
		strlcpy(rx->name, name, sizeof(rx->name));

	return SCMI_SUCCESS;
}

static void scmi_pinctrl_attributes(struct scmi_msg *msg)
{
	struct pin_attrs_tx {
		uint32_t identifier;
		uint32_t flags;
	} in_args = *(struct pin_attrs_tx *)msg->in;
	struct pin_attrs_rx return_values;

	unsigned nelems;
	unsigned nr_pins;
	const struct pinctrl_pin *pins;
	uint32_t selector = in_args.identifier;
	int ret;
	const unsigned *groups;
	const int *funcs;

	WARN("scmi: pinctrl_attributes id = %d flags = %d\n", in_args.identifier,
		 in_args.flags);

	if (msg->in_size != sizeof(in_args) || in_args.flags > 2) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	switch (in_args.flags) {
		case 0:
			ret = pinctrl_get_pins(&pins, &nr_pins);
			if (ret || selector > nr_pins) {
				ERROR("scmi: pinctrl_get_pins failed with ret = %d\n", ret);
				scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
			};
			return_values.status = set_rx(&return_values, pins[selector].name, nr_pins);
			break;
		case 1:
			if (selector > pinctrl_get_groups_count()) {
				ERROR("scmi: pinctrl_get_groups_count failed with sel = %d\n", selector);
				scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
			};

			ret = pinctrl_get_group_pins(selector, &groups, &nelems);
			if (ret) {
				ERROR("scmi: pinctrl_get_group_pins failed with ret = %d\n", ret);
				scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
			}

			return_values.status = set_rx(&return_values, pinctrl_get_group_name(selector), nelems);
			break;
		case 2:
			if (selector > pinctrl_get_functions_count()) {
				ERROR("scmi: pinctrl_get_functions_count failed with sel = %d\n",
					  selector);
				scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
			};

			ret = pinctrl_get_function_groups(selector, &funcs, &nelems);
			if (ret) {
				ERROR("scmi: pinctrl_get_fucntion_groups failed with ret = %d\n", ret);
				scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
			}

			return_values.status = set_rx(&return_values, pinctrl_get_function_name(selector), nelems);
			break;
		default:
			scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		}


	scmi_write_response(msg, &return_values, sizeof(return_values));
}

struct pin_assoc_tx {
	uint32_t identifier;
	uint32_t flags;
	uint32_t index;
};

struct pin_assoc_rx {
	int32_t status;
	uint32_t flags;
	uint16_t array[];
};

static uint32_t get_group_pins(struct pin_assoc_tx *tx, struct pin_assoc_rx *rx)
{
	uint32_t max_payload_size = SCMI_PLAYLOAD_MAX - sizeof(*rx);
	int counter = 0, lcounter = 0;

	const unsigned *pins;
	unsigned nr_pins;
	int ret;

	WARN("scmi: pinctrl get_group_pins sel = %d skip = %d\n", tx->identifier,
		 tx->index);

	ret = pinctrl_get_group_pins(tx->identifier, &pins, &nr_pins);
	if (ret) {
		ERROR("scmi: pinctrl_get_group_pins failed with ret = %d\n", ret);
		rx->status = SCMI_GENERIC_ERROR;
		return sizeof(rx->status);
	}

	for (counter = tx->index; counter < nr_pins; counter++, lcounter++) {
		if (lcounter * sizeof(uint16_t) >= max_payload_size) {
			break;
		}

		rx->array[lcounter] = pins[counter];
		WARN("group pins [%d] = %d\n", lcounter, rx->array[lcounter]);
		WARN("arr addr = %lx\n", (uint64_t)(&rx->array[lcounter]));
	}

	rx->flags = FLD(GENMASK(31, 16), nr_pins - counter) |
		FLD(GENMASK(11, 0), lcounter);
	WARN("scmi: pinctrl get nr_pins= %d cnt returned = %d sz=%zi\n", nr_pins,
	     lcounter,
	     sizeof(*rx) +
	     ALIGN_NEXT(lcounter * sizeof(uint16_t), sizeof(uint32_t)));
	rx->status = SCMI_SUCCESS;

	return sizeof(*rx) +
		ALIGN_NEXT(lcounter * sizeof(uint16_t), sizeof(uint32_t));
}

static uint32_t get_function_groups(struct pin_assoc_tx *tx, struct pin_assoc_rx *rx)
{
	uint32_t max_payload_size = SCMI_PLAYLOAD_MAX - sizeof(*rx);
	int counter = 0, lcounter = 0;

	const int *groups;
	unsigned nr_groups;
	int ret;

	WARN("scmi: pinctrl get_func_groups sel = %d skip = %d\n", tx->identifier,
		 tx->index);

	ret = pinctrl_get_function_groups(tx->identifier, &groups, &nr_groups);
	if (ret) {
		ERROR("scmi: pinctrl_get_fucntion_groups failed with ret = %d\n", ret);
		rx->status = SCMI_GENERIC_ERROR;
		return sizeof(rx->status);
	}
	// TODO amoi sort groups
	for (counter = tx->index; counter < nr_groups; counter++, lcounter++) {
		if (lcounter * sizeof(uint16_t) >= max_payload_size) {
			break;
		}

		rx->array[lcounter] = groups[counter];
		WARN("func groups [%d] = %d\n", lcounter, groups[counter]);
	}

	rx->flags = FLD(GENMASK(31, 16), nr_groups - counter) |
		FLD(GENMASK(11, 0), lcounter);

	WARN("scmi: pinctrl get nr_groups= %d cnt returned = %d\n", nr_groups,
		 lcounter);
	rx->status = SCMI_SUCCESS;

	return sizeof(*rx) +
		ALIGN_NEXT(lcounter * sizeof(uint16_t), sizeof(uint32_t));
};

static void scmi_list_associations(struct scmi_msg *msg)
{
	struct pin_assoc_tx tx = *(struct pin_assoc_tx *)msg->in;
	struct pin_assoc_rx rx;
	int rc;

	WARN("scmi: pinctrl_list_assoc id = %d flags = %d idx = %d\n", tx.identifier,
		tx.flags, tx.index);

	if (msg->in_size != sizeof(tx) || (tx.flags > 2)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	switch (tx.flags) {
	case 1:
		rc = get_group_pins(&tx, &rx);
		break;
	case 2:
		rc = get_function_groups(&tx, &rx);
		break;
	default:
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}
	scmi_write_response(msg, &rx, rc);
}

static void scmi_config_get(struct scmi_msg *msg)
{
	scmi_status_response(msg, SCMI_NOT_SUPPORTED);
}

static void scmi_config_set(struct scmi_msg *msg)
{
	scmi_status_response(msg, SCMI_NOT_SUPPORTED);
}

static void scmi_function_select(struct scmi_msg *msg)
{
	scmi_status_response(msg, SCMI_NOT_SUPPORTED);
}

static void scmi_request(struct scmi_msg *msg)
{ //done not tested
	struct request_tx {
		uint32_t identifier;
		uint32_t flags;
	} tx = *(struct request_tx *)msg->in;
	int ret;

	if (msg->in_size != sizeof(tx)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	if (tx.flags != 0) {
		WARN("Only pin request is supported\n");
		scmi_status_response(msg, SCMI_DENIED);
		return;
	}

	ret = pinctrl_request(tx.identifier);
	if (ret) {
		ERROR("scmi: pinctrl_request_pin failed with ret = %d\n", ret);
		scmi_status_response(msg, SCMI_INVALID_PARAMETERS);
		return;
	}
	//TODO amoi check all returns according to spec
	scmi_status_response(msg, SCMI_SUCCESS);
}

static void scmi_release(struct scmi_msg *msg)
{ //done not tested
	struct release_tx {
		uint32_t identifier;
		uint32_t flags;
	} tx = *(struct release_tx *)msg->in;
	int ret;

	if (msg->in_size != sizeof(tx)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}

	if (tx.flags != 0) {
		WARN("Only pin request is supported\n");
		scmi_status_response(msg, SCMI_DENIED);
		return;
	}

	ret = pinctrl_free(tx.identifier);
	if (ret) {
		ERROR("scmi: pinctrl_free_pin failed with ret = %d\n", ret);
		scmi_status_response(msg, SCMI_GENERIC_ERROR);
		return;
	}

	scmi_status_response(msg, SCMI_SUCCESS);
}
static void scmi_name_get(struct scmi_msg *msg)
{
	struct name_tx {
		uint32_t identifier;
		uint32_t flags;
	} tx = *(struct name_tx *)msg->in;
	struct name_rx {
		int32_t status;
		uint32_t flags;
		char name[64];
	} rx = {
			.status = SCMI_SUCCESS,
			.flags = 0,
	};
	int ret;
	unsigned nr_pins;
	const struct pinctrl_pin *pins;

	WARN("scmi: pinctrl_name_get id = %d flags = %d\n", tx.identifier,
		 tx.flags);

	if (msg->in_size != sizeof(tx) || (tx.flags > 2)) {
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}
	//TODO make defines
	switch (tx.flags) {
	case 0:
		ret = pinctrl_get_pins(&pins, &nr_pins);
		if (ret) {
			ERROR("scmi_name_get: unable to get pins: %d\n", ret);
			scmi_status_response(msg, SCMI_GENERIC_ERROR);
			return;
		}

		strlcpy(rx.name, pins[tx.identifier].name, sizeof(rx.name));
		break;
	case 1:
		strlcpy(rx.name, pinctrl_get_group_name(tx.identifier), sizeof(rx.name));
		break;
	case 2:
		strlcpy(rx.name, pinctrl_get_function_name(tx.identifier), sizeof(rx.name));
		break;
	default:
		scmi_status_response(msg, SCMI_PROTOCOL_ERROR);
		return;
	}
	scmi_write_response(msg, &rx, sizeof(rx));
}

static void scmi_set_permissions(struct scmi_msg *msg)
{
	scmi_status_response(msg, SCMI_NOT_SUPPORTED);
}

static const scmi_msg_handler_t scmi_pinctrl_handler_table[] = {
	[SCMI_PROTOCOL_VERSION] = report_version,
	[SCMI_PROTOCOL_ATTRIBUTES] = report_attributes,
	[SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] = report_message_attributes,
    [SCMI_PINCTRL_ATTRIBUTES] = scmi_pinctrl_attributes,
    [SCMI_PINCTRL_LIST_ASSOCIATIONS] = scmi_list_associations,
    [SCMI_PINCTRL_CONFIG_GET] = scmi_config_get,
    [SCMI_PINCTRL_CONFIG_SET] = scmi_config_set,
    [SCMI_PINCTRL_FUNCTION_SELECT] = scmi_function_select,
    [SCMI_PINCTRL_REQUEST] = scmi_request,
    [SCMI_PINCTRL_RELEASE] = scmi_release,
    [SCMI_PINCTRL_NAME_GET] = scmi_name_get,
    [SCMI_PINCTRL_SET_PERMISSIONS] = scmi_set_permissions
};

scmi_msg_handler_t scmi_msg_get_pinctrl_handler(struct scmi_msg *msg)
{
	unsigned int message_id = SPECULATION_SAFE_VALUE(msg->message_id);

	if (message_id >= ARRAY_SIZE(scmi_pinctrl_handler_table)) {
		VERBOSE("Pinctrl domain handle not found %u\n", msg->message_id);
		return NULL;
	}

	return scmi_pinctrl_handler_table[message_id];
}

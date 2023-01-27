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
#include <lib/mmio.h>
#include <drivers/pinctrl.h>
#include "rcar_private.h"
#include "rcar_scmi_resources.h"

enum scmi_message_id {
  PROTOCOL_VERSION = 0x0,
  PROTOCOL_ATTRIBUTES,
  PROTOCOL_MESSAGE_ATTRIBUTES,
  PINCTRL_ATTRIBUTES = 0x3,
  PINCTRL_LIST_ASSOCIATIONS = 0x4,
  PINCTRL_CONFIG_GET = 0x5,
  PINCTRL_CONFIG_SET = 0x6,
  PINCTRL_FUNCTION_SELECT = 0x7,
  PINCTRL_REQUEST = 0x8,
  PINCTRL_RELEASE = 0x9,
  PINCTRL_NAME_GET = 0xa,
  PINCTRL_SET_PERMISSIONS = 0xb,
  SCMI_LAST_ID
};

static const struct scmi_pinctrl default_device = {
	.groups = (int[]){-1},
	.pins = (int[]){-1},
};

struct scmi_pinctrl rcar_pinctrl[RCAR_PINCTRL_MAX] = {
	[0 ... RCAR_PINCTRL_MAX -1 ] = default_device,
};

static uint32_t protocol_version(size_t channel __unused,
				 volatile uint8_t *param,
				 size_t size __unused)
{// done
	int32_t *status = (int32_t*)param;
	uint32_t *version = (uint32_t*)(param + sizeof(*status));
	*status = SCMI_SUCCESS;
	*version = 0x10000;
	WARN("scmi: Process pinctrl protocol_version\n");
	return sizeof(*status) + sizeof(*version);
}

static uint32_t protocol_attrs(size_t channel __unused,
			       volatile uint8_t *param,
			       size_t size __unused)
{ //done not tested
	struct attrs_rx {
		int32_t status;
		uint32_t attrs_low;
		uint32_t attrs_high;
	} *rx = (struct attrs_rx *)param;

	const struct pinctrl_pin *pins;
	unsigned nr_pins;
	int ret;

	uint16_t nr_groups = pinctrl_get_groups_count();
	uint16_t nr_functions = pinctrl_get_functions_count();

	ret = pinctrl_get_pins(&pins, &nr_pins);
	if (ret) {
		ERROR("scmi: pinctrl_get_pins failed with ret = %d\n", ret);
		rx->status = SCMI_GENERIC_ERROR;
		return sizeof(rx->status);
	};

	rx->status = SCMI_SUCCESS;
	rx->attrs_low = FLD(GENMASK(31, 16), nr_groups) |
		 FLD(GENMASK(15,  0), nr_pins);
	rx->attrs_high = FLD(GENMASK(15, 0), nr_functions);

	WARN("scmi: Process pinctrl protocol_attrs group_max = %d, func_max = %d\n",
			nr_groups, nr_functions);
	return sizeof(*rx);
}

static uint32_t protocol_msg_attrs(size_t channel __unused,
				   volatile uint8_t *param,
				   size_t size)
{ //done tested
	uint32_t id = *(uint32_t*)param;
	int32_t *status = (int32_t*)param;
	uint32_t *attrs = (uint32_t*)(param + sizeof(*status));

	if (size != sizeof(id)) {
		*status = SCMI_PROTOCOL_ERROR;
		return sizeof(*status);
	}

	/* all commands are mandatory */
	*status = SCMI_SUCCESS;
	*attrs = 0x0;

	WARN("scmi: Process pinctrl protocol_msg_attrs\n");

	return sizeof(*status) + sizeof(*attrs);
}

struct pin_attrs_rx {
	int32_t status;
	uint32_t attributes;
	char name[16];
};

static int set_rx(struct pin_attrs_rx *rx, const char *name)
{
	assert(name != NULL);

	rx->attributes = 0;
	if (sizeof(name) > sizeof(rx->name))
		rx->attributes = FLD(31, 1);
	else
		strlcpy(rx->name, name, sizeof(rx->name));

	return SCMI_SUCCESS;
}

static uint32_t scmi_pinctrl_attributes(size_t channel __unused,
                                   volatile uint8_t *param, size_t size)
{ //done not tested
	struct pin_attrs_tx {
		uint32_t identifier;
		uint32_t flags;
	} tx = *(struct pin_attrs_tx *)param;
	struct pin_attrs_rx *rx = (struct pin_attrs_rx *)param;

	const struct pinctrl_pin *pins;
	unsigned nr_pins;
	uint32_t selector = tx.identifier;
	int ret;

	WARN("scmi: pinctrl_attributes id = %d flags = %d\n", tx.identifier,
		 tx.flags);

	if (size != sizeof(tx) || (tx.flags > 2)) {
		rx->status = SCMI_PROTOCOL_ERROR;
		return sizeof(rx->status);
	}

	switch (tx.flags) {
	case 0:
		ret = pinctrl_get_pins(&pins, &nr_pins);
		if (ret || selector > nr_pins) {
			ERROR("scmi: pinctrl_get_pins failed with ret = %d\n", ret);
			rx->status = SCMI_GENERIC_ERROR;
			return sizeof(rx->status);
		};
		rx->status = set_rx(rx, pins[selector].name);
		break;
	case 1:
		//group
		if (selector > pinctrl_get_groups_count()) {
			ERROR("scmi: pinctrl_get_groups_count failed with sel = %d\n", selector);
			rx->status = SCMI_GENERIC_ERROR;
			return sizeof(rx->status);
		};
		rx->status = set_rx(rx, pinctrl_get_group_name(selector));
		break;
	case 2:
		// function
		if (selector > pinctrl_get_functions_count()) {
			ERROR("scmi: pinctrl_get_functions_count failed with sel = %d\n",
				  selector);
			rx->status = SCMI_GENERIC_ERROR;
			return sizeof(rx->status);
		};
		rx->status = set_rx(rx, pinctrl_get_function_name(selector));
		break;
	default:
		rx->status = SCMI_PROTOCOL_ERROR;
		return sizeof(rx->status);
	}

	return sizeof(*rx);
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

static uint32_t get_group_pins(struct pin_assoc_tx *tx,
                               struct pin_assoc_rx *rx)
{ //done not tested
	uint32_t max_payload_size = SCMI_MAX_PAYLOAD - sizeof(*rx);
	int counter = 0, lcounter = 0;

	const unsigned *pins;
	unsigned nr_pins;
	int ret;

	WARN("scmi: pinctrl get_group_pins sel = %d skip = %d\n", tx->identifier,
		 tx->index);

	ret = pinctrl_get_group_pins(tx->index, &pins, &nr_pins);
	if (ret) {
		ERROR("scmi: pinctrl_get_group_pins failed with ret = %d\n", ret);
		rx->status = SCMI_GENERIC_ERROR;
		return sizeof(rx->status);
	}
	//todo amoi sort pins asc

	for (counter = tx->index; counter < nr_pins; counter++, lcounter++) {
		if (lcounter * sizeof(uint16_t) >= max_payload_size) {
			break;
		}

		rx->array[lcounter] = pins[counter];
		WARN("group pins [%d] = %d\n", lcounter, pins[counter]);
	}

	rx->flags = FLD(GENMASK(31, 16), nr_pins - counter) |
		FLD(GENMASK(11, 0), lcounter);
	WARN("scmi: pinctrl get nr_pins= %d cnt returned = %d\n", nr_pins,
		 lcounter);
	rx->status = SCMI_SUCCESS;

	return sizeof(*rx) +
		ALIGN_NEXT(lcounter * sizeof(uint16_t), sizeof(uint32_t));
}

static uint32_t get_function_groups(struct pin_assoc_tx *tx,
                                    struct pin_assoc_rx *rx)
{ //done not tested
	uint32_t max_payload_size = SCMI_MAX_PAYLOAD - sizeof(*rx);
	int counter = 0, lcounter = 0;

	const int *groups;
	unsigned nr_groups;
	int ret;

	WARN("scmi: pinctrl get_func_groups sel = %d skip = %d\n", tx->identifier,
		 tx->index);

	ret = pinctrl_get_function_groups(tx->index, &groups, &nr_groups);
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

static uint32_t scmi_list_associations(size_t channel __unused,
				  volatile uint8_t *param, size_t size __unused)
{ //done not tested
	struct pin_assoc_tx tx = *(struct pin_assoc_tx *)param;
	struct pin_assoc_rx *rx = (struct pin_assoc_rx *)param;
	int rc = 0;

	WARN("scmi: pinctrl_list_assoc id = %d flags = %d idx = %d\n", tx.identifier,
		tx.flags, tx.index);

	if (size != sizeof(tx) || (tx.flags > 2)) {
		rx->status = SCMI_PROTOCOL_ERROR;
		return sizeof(rx->status);
	}

	switch (tx.flags) {
	case 1:
		//groups
		rc = get_group_pins(&tx, rx);
		break;
	case 2:
		//function
		rc = get_function_groups(&tx, rx);
		break;
	default:
		rx->status = SCMI_PROTOCOL_ERROR;
		return sizeof(rx->status);
	}

	return rc;
}

struct config_tx {
	uint32_t identifier;
	uint32_t attributes;
};
struct config_rx {
	int32_t status;
	uint32_t config_value;
};

static uint32_t get_config(struct config_tx *tx, struct config_rx *rx)
{ //done not tested
	int ret;
	unsigned long config_param;

	config_param = FLD(GENMASK(7, 0), tx->attributes);

	ret = pinctrl_config_get(tx->identifier, &config_param);
	if (ret) {
		ERROR("scmi: pinctrl_config_get failed with ret = %d\n", ret);
		rx->status = SCMI_GENERIC_ERROR;
		return sizeof(rx->status);
	}

	rx->config_value = config_param;
	rx->status = SCMI_SUCCESS;
	return sizeof(*rx);
}

static uint32_t get_config_group(struct config_tx *tx, struct config_rx *rx)
{ //done not tested
	int ret;
	unsigned long config_param;

	config_param = FLD(GENMASK(7, 0), tx->attributes);

	ret = pinctrl_config_group_get(tx->identifier, &config_param);
	if (ret) {
		ERROR("scmi: pinctrl_config_group_get failed with ret = %d\n", ret);
		rx->status = SCMI_GENERIC_ERROR;
		return sizeof(rx->status);
	}

	rx->config_value = config_param;
	rx->status = SCMI_SUCCESS;
	return sizeof(*rx);
}

static uint32_t scmi_config_get(size_t channel __unused,
           volatile uint8_t *param, size_t size __unused)
{ // done not tested
	struct config_tx tx = *(struct config_tx *)param;
	struct config_rx *rx = (struct config_rx *)param;
	int rc = 0;
	int selector;

	WARN("scmi: config_get id = %d attribs = %d\n", tx.identifier,
		 tx.attributes);

	if (size != sizeof(tx)) {
		rx->status = SCMI_PROTOCOL_ERROR;
		return sizeof(rx->status);
	}

	selector = FLD_GET(GENMASK(9, 8), tx.attributes);

	switch (selector) {
	case 0:
		// pin
		rc = get_config(&tx, rx);
		break;
	case 1:
		// group
		rc = get_config_group(&tx, rx);
		break;
	default:
		rx->status = SCMI_PROTOCOL_ERROR;
		return sizeof(rx->status);
	}

	return rc;
}

struct config_set_tx {
	uint32_t identifier;
	uint32_t attributes;
};

struct config_set_rx {
	int32_t status;
};

static uint32_t set_config(struct config_set_tx *tx, struct config_set_rx *rx)
{ //done not tested
	int ret;
	unsigned long config_param;

	config_param = FLD(GENMASK(7, 0), tx->attributes);

	ret = pinctrl_config_set(tx->identifier, config_param);
	if (ret) {
		ERROR("scmi: pinctrl_config_set failed with ret = %d\n", ret);
		rx->status = SCMI_GENERIC_ERROR;
		return sizeof(rx->status);
	}

	rx->status = SCMI_SUCCESS;
	return sizeof(*rx);
}

static uint32_t set_config_group(struct config_set_tx *tx, struct config_set_rx *rx)
{ //done not tested
  int ret;
  unsigned long config_param;

  config_param = FLD(GENMASK(7, 0), tx->attributes);

  ret = pinctrl_config_group_set(tx->identifier, config_param);
  if (ret) {
    ERROR("scmi: pinctrl_config_group_set failed with ret = %d\n", ret);
    rx->status = SCMI_GENERIC_ERROR;
    return sizeof(rx->status);
  }

  rx->status = SCMI_SUCCESS;
  return sizeof(*rx);
}

static uint32_t scmi_config_set(size_t channel __unused,
                                volatile uint8_t *param, size_t size __unused)
{ // done not tested
	struct config_set_tx tx = *(struct config_set_tx *)param;
	struct config_set_rx *rx = (struct config_set_rx *)param;
	int rc = 0;
	int selector;

	WARN("scmi: config_set id = %d attribs = %d\n", tx.identifier, tx.attributes);

	if (size != sizeof(tx)) {
		rx->status = SCMI_PROTOCOL_ERROR;
		return sizeof(rx->status);
	}

	selector = FLD_GET(GENMASK(9, 8), tx.attributes);

	switch (selector) {
	case 0:
		// pin
		rc = set_config(&tx, rx);
		break;
	case 1:
		// group
		rc = set_config_group(&tx, rx);
		break;
	default:
		rx->status = SCMI_PROTOCOL_ERROR;
		return sizeof(rx->status);
	}

	return rc;
}

//former set_mux
static uint32_t scmi_function_select(size_t channel __unused,
                                     volatile uint8_t *param,
                                     size_t size __unused)
{ //done not tested
	struct mux_tx {
		uint32_t identifier;
		uint32_t function_id;
		uint32_t flags;
	} tx = *(struct mux_tx *)param;
	int32_t *status = (int32_t *)param;
	int ret;

	WARN("scmi:pinctrl set mux id = %d, func = %d flags = %d\n",
		 tx.identifier, tx.function_id, tx.flags);

	if (size != sizeof(tx)) {
		*status = SCMI_PROTOCOL_ERROR;
		return sizeof(*status);
	}

	if (tx.flags == 0) {
		WARN("Set function to pin is not supported\n");
		*status = SCMI_NOT_SUPPORTED;
		return sizeof(*status);
	}

	ret = pinctrl_set_mux(tx.function_id, tx.identifier);
	if (ret) {
		ERROR("scmi: pinctrl_set_mux failed with ret = %d\n", ret);
		*status = SCMI_GENERIC_ERROR;
		return sizeof(*status);
	}

	*status = SCMI_SUCCESS;
	return sizeof(*status);
}

static uint32_t scmi_request(size_t channel __unused, volatile uint8_t *param,
                            size_t size __unused)
{ //done not tested
	struct request_tx {
		uint32_t identifier;
		uint32_t flags;
	} tx = *(struct request_tx *)param;
	int32_t *status = (int32_t *)param;
	int ret;

	if (size != sizeof(tx)) {
		*status = SCMI_PROTOCOL_ERROR;
		return sizeof(*status);
	}

	if (tx.flags != 0) {
		WARN("Only pin request is supported\n");
		*status = SCMI_DENIED;
		return sizeof(*status);
	}

	ret = pinctrl_request(tx.identifier);
	if (ret) {
		ERROR("scmi: pinctrl_request_pin failed with ret = %d\n", ret);
		*status = SCMI_INVALID_PARAMETERS;
		return sizeof(*status);
	}
	//TODO amoi check all returns according to spec
	*status = SCMI_SUCCESS;
	return sizeof(*status);
}

static uint32_t scmi_release(size_t channel __unused, volatile uint8_t *param,
                             size_t size __unused)
{ //done not tested
	struct release_tx {
		uint32_t identifier;
		uint32_t flags;
	} tx = *(struct release_tx *)param;
	int32_t *status = (int32_t *)param;
	int ret;

	if (size != sizeof(tx)) {
		*status = SCMI_PROTOCOL_ERROR;
		return sizeof(*status);
	}

	if (tx.flags != 0) {
		WARN("Only pin request is supported\n");
		*status = SCMI_DENIED;
		return sizeof(*status);
	}

	ret = pinctrl_free(tx.identifier);
	if (ret) {
		ERROR("scmi: pinctrl_free_pin failed with ret = %d\n", ret);
		*status = SCMI_GENERIC_ERROR;
		return sizeof(*status);
	}

	*status = SCMI_SUCCESS;
	return sizeof(*status);
}

static uint32_t scmi_name_get(size_t channel __unused, volatile uint8_t *param,
                             size_t size __unused)
{ //done not tested
	struct name_tx {
		uint32_t identifier;
		uint32_t flags;
	} tx = *(struct name_tx *)param;
	struct name_rx {
		int32_t status;
		uint32_t flags;
		char name[64];
	} *rx = (struct name_rx *)param;
	int ret;
	unsigned nr_pins;
	const struct pinctrl_pin *pins;

	WARN("scmi: pinctrl_name_get id = %d flags = %d\n", tx.identifier,
		 tx.flags);

	if (size != sizeof(tx) || (tx.flags > 2)) {
		rx->status = SCMI_PROTOCOL_ERROR;
		return sizeof(rx->status);
	}

	switch (tx.flags) {
	case 0: //pin
		ret = pinctrl_get_pins(&pins, &nr_pins);
		if (ret) {
			ERROR("scmi_name_get: unable to get pins: %d\n", ret);
			rx->status = SCMI_GENERIC_ERROR;
			return sizeof(rx->status);
		}

		strlcpy(rx->name, pins[tx.identifier].name, sizeof(rx->name));
		break;
	case 1: //group
		strlcpy(rx->name, pinctrl_get_group_name(tx.identifier), sizeof(rx->name));
		break;
	case 2: //function
		strlcpy(rx->name, pinctrl_get_function_name(tx.identifier),
                  sizeof(rx->name));
		break;
	default:
		rx->status = SCMI_PROTOCOL_ERROR;
		return sizeof(rx->status);
	}

	rx->status = SCMI_SUCCESS;
	return sizeof(*rx);
}

static uint32_t scmi_set_permissions(size_t channel __unused, volatile uint8_t *param,
                         size_t size __unused)
{
	struct perms_tx {
		uint32_t agent_id;
		uint32_t identifier;
		uint32_t flags;
	} tx = *(struct perms_tx *)param;
	int32_t *status = (int32_t *)param;

	WARN("set permissions agent_id %d, id= %d, flags = %d\n", tx.agent_id,
		 tx.identifier, tx.flags);
	*status = SCMI_NOT_SUPPORTED;
	return sizeof(*status);
}

//====== end if impl
  /* static uint32_t get_pins(size_t channel __unused, volatile uint8_t * param, */
  /*                          size_t size __unused) { */
  /*   uint32_t skip = *(uint32_t *)param; */
  /*   struct pins_rx { */
  /*     int32_t status; */
  /*     uint32_t nr_pins; */
  /*     uint16_t pins[]; */
  /*   } *rx = (struct pins_rx *)param; */
  /*   uint32_t max_payload_size = SCMI_MAX_PAYLOAD - sizeof(*rx); */
  /*   int counter = 0, lcounter = 0; */

  /*   const struct pinctrl_pin *pins; */
  /*   unsigned nr_pins; */
  /*   int ret; */

  /*   WARN("scmi: pinctrl get_pins skip = %d\n", skip); */
  /*   if (size != sizeof(skip)) { */
  /*     rx->status = SCMI_PROTOCOL_ERROR; */
  /*     return sizeof(rx->status); */
  /*   } */

  /*   ret = pinctrl_get_pins(&pins, &nr_pins); */
  /*   if (ret) { */
  /*     ERROR("scmi: pinctrl_get_pins failed with ret = %d\n", ret); */
  /*     rx->status = SCMI_GENERIC_ERROR; */
  /*     return sizeof(rx->status); */
  /*   }; */

  /*   WARN("max_payload_size = %d\n", max_payload_size); */

  /*   for (counter = skip; counter < nr_pins; counter++, lcounter++) { */
  /*     const struct pinctrl_pin *info = &pins[counter]; */
  /*     if (lcounter * sizeof(uint16_t) >= max_payload_size) { */
  /*       break; */
  /*     } */

  /*     rx->pins[lcounter] = info->pin != (uint16_t)-1 ? info->pin : counter; */
  /*   } */

  /*   rx->nr_pins = lcounter; */
  /*   WARN("scmi: pinctrl get nr_pins = %d\n", rx->nr_pins); */
  /*   rx->status = SCMI_SUCCESS; */

  /*   return sizeof(*rx) + */
  /*          ALIGN_NEXT(lcounter * sizeof(uint16_t), sizeof(uint32_t)); */
  /* } */

typedef uint32_t (*pinctrl_handler_t)(size_t, volatile uint8_t *, size_t);

static pinctrl_handler_t pinctrl_handlers[SCMI_LAST_ID] = {
    [PROTOCOL_VERSION] = protocol_version,
    [PROTOCOL_ATTRIBUTES] = protocol_attrs,
    [PROTOCOL_MESSAGE_ATTRIBUTES] = protocol_msg_attrs,
    [PINCTRL_ATTRIBUTES] = scmi_pinctrl_attributes,
    [PINCTRL_LIST_ASSOCIATIONS] = scmi_list_associations,
    [PINCTRL_CONFIG_GET] = scmi_config_get,
    [PINCTRL_CONFIG_SET] = scmi_config_set,
    [PINCTRL_FUNCTION_SELECT] = scmi_function_select,
    [PINCTRL_REQUEST] = scmi_request,
    [PINCTRL_RELEASE] = scmi_release,
    [PINCTRL_NAME_GET] = scmi_name_get,
    [PINCTRL_SET_PERMISSIONS] = scmi_set_permissions
};

uint32_t rcar_scmi_handle_pinctrl(size_t channel, uint8_t cmd,
					volatile uint8_t *param, size_t size)
{
	if (cmd >= SCMI_LAST_ID) {
		*(int32_t *)param = SCMI_NOT_SUPPORTED;
		return sizeof(int32_t);
	}

	assert(pinctrl_handlers[cmd] != NULL);

	return pinctrl_handlers[cmd](channel, param, size);
}

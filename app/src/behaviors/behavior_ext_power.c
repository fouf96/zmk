/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_ext_power

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <drivers/behavior.h>
#include <drivers/ext_power.h>

#include <dt-bindings/zmk/ext_power.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct behavior_ext_power_config {
  char *power_node_name;
};

static int
on_keymap_binding_convert_central_state_dependent_params(struct zmk_behavior_binding *binding,
                                                         struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_ext_power_config *cfg = dev->config;

    const struct device *ext_power = device_get_binding(cfg->power_node_name);
    if (ext_power == NULL) {
        LOG_ERR("Unable to retrieve ext_power device: %d", binding->param1);
        return -EIO;
    }

    if (binding->param1 == EXT_POWER_TOGGLE_CMD) {
        binding->param1 = ext_power_get(ext_power) > 0 ? EXT_POWER_OFF_CMD : EXT_POWER_ON_CMD;
    }

    return 0;
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_ext_power_config *cfg = dev->config;

    const struct device *ext_power = device_get_binding(cfg->power_node_name);
    if (ext_power == NULL) {
        LOG_ERR("Unable to retrieve ext_power device: %d", binding->param1);
        return -EIO;
    }

    switch (binding->param1) {
    case EXT_POWER_OFF_CMD:
        return ext_power_disable(ext_power);
    case EXT_POWER_ON_CMD:
        return ext_power_enable(ext_power);
    case EXT_POWER_TOGGLE_CMD:
        if (ext_power_get(ext_power) > 0)
            return ext_power_disable(ext_power);
        else
            return ext_power_enable(ext_power);
    default:
        LOG_ERR("Unknown ext_power command: %d", binding->param1);
    }

    return -ENOTSUP;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static int behavior_ext_power_init(const struct device *dev) { return 0; };

// ? do we have to put the api into the KP_INST(n) as well?
static const struct behavior_driver_api behavior_ext_power_driver_api = {
    .binding_convert_central_state_dependent_params =
        on_keymap_binding_convert_central_state_dependent_params,
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
    .locality = BEHAVIOR_LOCALITY_GLOBAL,
};

#define KP_INST(n)   
    static struct behavior_ext_power_config behavior_ext_power_config_#nn = {
    .power_node_name = DT_INST_PROP_OR(n, power_node_name, "EXT_POWER")
    };

    BEHAVIOR_DT_INST_DEFINE(n, behavior_ext_power_init, NULL, NULL, &behavior_ext_power_config_#nn, POST_KERNEL,
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_ext_power_driver_api);

DT_INST_FOREACH_STATUS_OKAY(KP_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */

// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "powerbtn.h"

#include <stdio.h>

#include <acpica/acpi.h>
#include <magenta/syscalls.h>
#include <pthread.h>

/**
 * @brief  Handle the Power Button Fixed Event
 *
 * We simply write to a well known port. A user-mode driver should pick
 * this event and take action.
 */
static uint32_t power_button_object_handler(void* ctx) {
    mx_handle_t event = (mx_handle_t)(uintptr_t)ctx;
    mx_object_signal(event, MX_SIGNAL_SIGNALED, 0);
    // Note that the spec indicates to return 0. The code in the
    // Intel implementation (AcpiEvFixedEventDetect) reads differently.
    return ACPI_INTERRUPT_HANDLED;
}

static void notify_object_handler(ACPI_HANDLE Device, UINT32 Value, void* Context) {
    ACPI_DEVICE_INFO *info = NULL;
    ACPI_STATUS status = AcpiGetObjectInfo(Device, &info);
    if (status != AE_OK) {
        if (info) {
            ACPI_FREE(info);
        }
        return;
    }

    mx_handle_t event = (mx_handle_t)(uintptr_t)Context;

    // Handle powerbutton events via the notify interface
    bool power_btn = false;
    if (info->Valid & ACPI_VALID_HID) {
        if (Value == 128 &&
            !strncmp(info->HardwareId.String, "PNP0C0C", info->HardwareId.Length)) {

            power_btn = true;
        } else if (Value == 199 &&
                   (!strncmp(info->HardwareId.String, "MSHW0028", info->HardwareId.Length) ||
                    !strncmp(info->HardwareId.String, "MSHW0040", info->HardwareId.Length))) {
            power_btn = true;
        }
    }

    if (power_btn) {
        mx_object_signal(event, MX_SIGNAL_SIGNALED, 0);
    }

    ACPI_FREE(info);
}

static void acpi_poweroff(void) {
    ACPI_STATUS status = AcpiEnterSleepStatePrep(5);
    if (status == AE_OK) {
        AcpiEnterSleepState(5);
    }
}

static void* power_button_thread(void* arg) {
    mx_handle_t event = (mx_handle_t)(uintptr_t)arg;

    for(;;) {
        mx_signals_state_t state;
        mx_status_t status = mx_handle_wait_one(event,
                                                MX_SIGNAL_SIGNALED,
                                                MX_TIME_INFINITE,
                                                &state);
        if (status != NO_ERROR) {
            continue;
        }
        if (state.satisfied != MX_SIGNAL_SIGNALED) {
            continue;
        }
        acpi_poweroff();
    }

    printf("acpi power button thread terminated\n");
    return NULL;
}

mx_status_t install_powerbtn_handlers(void) {
    // Hacks to make the power button power off the machine

    mx_handle_t power_button_event = mx_event_create(0);
    if (power_button_event < 0) {
        return power_button_event;
    }

    ACPI_STATUS status = AcpiInstallFixedEventHandler(ACPI_EVENT_POWER_BUTTON,
                                          power_button_object_handler,
                                          (void*)(uintptr_t)power_button_event);
    if (status != AE_OK) {
        return ERR_INTERNAL;
    }

    AcpiInstallNotifyHandler(ACPI_ROOT_OBJECT, ACPI_SYSTEM_NOTIFY | ACPI_DEVICE_NOTIFY, notify_object_handler, (void*)(uintptr_t)power_button_event);

    pthread_t thread;
    mx_status_t mx_status = pthread_create(&thread, NULL, power_button_thread, (void*)(uintptr_t)power_button_event);
    if (mx_status != NO_ERROR) {
        return mx_status;
    }
    pthread_detach(thread);
    return NO_ERROR;
}
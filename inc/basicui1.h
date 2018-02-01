#ifndef __basicui1_H__
#define __basicui1_H__

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>

#include <sensor.h>
#include <stdio.h> // à cause de sprintf pour convertir float to char
#include <app_common.h>
#include <device/haptic.h>
#include <curl/curl.h>
#include <device/power.h>


#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "basicui1"

#if !defined(PACKAGE)
#define PACKAGE "org.example.basicui1"
#endif

#endif /* __basicui1_H__ */

void clicked_cb(void *data, Evas_Object *obj, void *event_info);

/*
obs-websocket
Copyright (C) 2016-2021 Stephane Lepin <stephane.lepin@gmail.com>
Copyright (C) 2020-2021 Kyle Manning <tt2468@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#ifndef _OBS_WEBSOCKET_H
#define _OBS_WEBSOCKET_H

#include <obs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* obs_websocket_vendor;
typedef void (*obs_websocket_request_callback)(obs_data_t *request_data, obs_data_t *response_data, void *priv);

inline proc_handler_t *ph;

inline proc_handler_t *obs_websocket_get_ph(void)
{
	proc_handler_t *global_ph = obs_get_proc_handler();
	assert(global_ph != NULL);

	calldata_t cd = {0};
	if (!proc_handler_call(global_ph, "obs_websocket_api_get_ph", &cd))
		blog(LOG_DEBUG, "Unable to fetch obs-websocket proc handler object. obs-websocket not installed?");
	proc_handler_t *ret = (proc_handler_t*)calldata_ptr(&cd, "ph");
	calldata_free(&cd);

	return ret;
}

inline bool obs_websocket_run_simple_proc(obs_websocket_vendor vendor, const char *proc_name, calldata_t *cd)
{
	if (!ph || !vendor || !proc_name || !*proc_name || !cd)
		return false;

	calldata_set_ptr(cd, "vendor", vendor);

	proc_handler_call(ph, proc_name, cd);
	return calldata_bool(cd, "success");
}

// ALWAYS CALL VIA `obs_module_post_load()` CALLBACK!
inline obs_websocket_vendor obs_websocket_register_vendor(const char *vendor_name)
{
	ph = obs_websocket_get_ph();
	if (!ph)
		return NULL;

	calldata_t cd = {0};

	calldata_set_string(&cd, "vendor_name", vendor_name);

	proc_handler_call(ph, "vendor_create", &cd);
	obs_websocket_vendor ret = calldata_ptr(&cd, "vendor");
	calldata_free(&cd);

	return ret;
}

inline bool obs_websocket_register_request(obs_websocket_vendor vendor, const char *request_name, obs_websocket_request_callback request_callback, void* priv_data)
{
	calldata_t cd = {0};

	calldata_set_string(&cd, "name", request_name);
	calldata_set_ptr(&cd, "callback", (void*)request_callback);
	calldata_set_ptr(&cd, "callback_priv_data", priv_data);

	bool success = obs_websocket_run_simple_proc(vendor, "vendor_request_register", &cd);
	calldata_free(&cd);

	return success;
}

inline bool obs_websocket_unregister_request(obs_websocket_vendor vendor, const char *request_name, obs_websocket_request_callback request_callback, void* priv_data)
{
	calldata_t cd = {0};

	calldata_set_string(&cd, "name", request_name);
	calldata_set_ptr(&cd, "callback", (void*)request_callback);
	calldata_set_ptr(&cd, "callback_priv_data", priv_data);

	bool success = obs_websocket_run_simple_proc(vendor, "vendor_request_unregister", &cd);
	calldata_free(&cd);

	return success;
}

inline bool obs_websocket_api_emit_event(obs_websocket_vendor vendor, const char *event_name, obs_data_t *event_data)
{
	calldata_t cd = {0};

	calldata_set_string(&cd, "event_name", event_name);
	calldata_set_ptr(&cd, "event_data", (void*)event_data);

	bool success = obs_websocket_run_simple_proc(vendor, "vendor_event_emit", &cd);
	calldata_free(&cd);

	return success;
}

#ifdef __cplusplus
}
#endif

#endif
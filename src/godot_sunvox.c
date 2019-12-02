#include <dlfcn.h>
#include <stdlib.h>

#include <gdnative_api_struct.gen.h>

#define SUNVOX_MAIN
#include "sunvox.h"

typedef struct user_data_struct {
    bool init;
    int slot;
} user_data_struct;

int sunvox_dll_loaded = 0;

const godot_gdnative_core_api_struct *api = NULL;
const godot_gdnative_ext_nativescript_api_struct *nativescript_api = NULL;

GDCALLINGCONV void *sunvox_constructor(godot_object *p_instance, void *p_method_data) {
    user_data_struct* user_data = (user_data_struct*) api->godot_alloc(sizeof(user_data_struct));
    user_data->init = false;
    user_data->slot = 0;

    if (!sunvox_dll_loaded++) {
        if (sv_load_dll()) {
            api->godot_print_error("sv_load_dll() failed", __func__, __FILE__, __LINE__);
        } else {
            int ver = sv_init("", 44100, 2, 0 );
            if (ver >= 0) {
            } else {
                api->godot_print_error("sv_init() error", __func__, __FILE__, __LINE__);
            }
        }
    }

    return user_data;
}

GDCALLINGCONV void sunvox_destructor(godot_object *p_instance, void *p_method_data, void *p_user_data) {
    user_data_struct* user_data = (user_data_struct*) p_user_data;

    if (user_data->init) {
	sv_stop(user_data->slot);
	sv_close_slot(user_data->slot);
	sv_deinit();
    }

    if (!--sunvox_dll_loaded) {
	sv_unload_dll();
    }

    api->godot_free(p_user_data);
}

godot_variant sunvox_init(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
    user_data_struct* user_data = (user_data_struct*) p_user_data;
    godot_variant ret;
    int slot = api->godot_variant_as_int(p_args[0]);	

    int res = sv_open_slot(slot);
    user_data->init = true;
    user_data->slot = slot;

    api->godot_variant_new_int(&ret, res);
    return ret;
}

godot_variant sunvox_load(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
    user_data_struct* user_data = (user_data_struct*) p_user_data;
    godot_variant ret;
    godot_string file_name = api->godot_variant_as_string(p_args[0]);
    godot_char_string char_str = api->godot_string_ascii(&file_name);

    int res = sv_load(user_data->slot, api->godot_char_string_get_data(&char_str));

    api->godot_variant_new_int(&ret, res);
    return ret;
}

godot_variant sunvox_load_from_memory(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
    user_data_struct* user_data = (user_data_struct*) p_user_data;
    godot_variant ret;
    godot_pool_byte_array data = api->godot_variant_as_pool_byte_array(p_args[0]);

    godot_int size = api->godot_pool_byte_array_size(&data);
    godot_pool_byte_array_read_access* p_read = api->godot_pool_byte_array_read(&data);
    int res = sv_load_from_memory(user_data->slot, (void*) api->godot_pool_byte_array_read_access_ptr(p_read), size);

    api->godot_variant_new_int(&ret, res);
    return ret;
}

godot_variant sunvox_play_from_beginning(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
    user_data_struct* user_data = (user_data_struct*) p_user_data;
    godot_variant ret;

    int res = sv_play_from_beginning(user_data->slot);

    api->godot_variant_new_int(&ret, res);
    return ret;
}

godot_variant sunvox_send_event(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
    user_data_struct* user_data = (user_data_struct*) p_user_data;
    godot_variant ret;
    int track_num = api->godot_variant_as_int(p_args[0]);
    int note = api->godot_variant_as_int(p_args[1]);
    int vel = api->godot_variant_as_int(p_args[2]);
    int module = api->godot_variant_as_int(p_args[3]);
    int ctl = api->godot_variant_as_int(p_args[4]);
    int ctl_val = api->godot_variant_as_int(p_args[5]);

    int res = sv_send_event(user_data->slot, track_num, note, vel, module, ctl, ctl_val);

    api->godot_variant_new_int(&ret, res);
    return ret;
}

godot_variant sunvox_volume(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
    user_data_struct* user_data = (user_data_struct*) p_user_data;
    godot_variant ret;
    int vol = api->godot_variant_as_int(p_args[0]);

    int res = sv_volume(user_data->slot, vol);

    api->godot_variant_new_int(&ret, res);
    return ret;
}

void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *p_options) {
    api = p_options->api_struct;

    // Now find our extensions.
    for (int i = 0; i < api->num_extensions; i++) {
        switch (api->extensions[i]->type) {
            case GDNATIVE_EXT_NATIVESCRIPT: {
                nativescript_api = (godot_gdnative_ext_nativescript_api_struct *)api->extensions[i];
            }; break;
            default: break;
        }
    }
}

void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *p_options) {
    api = NULL;
    nativescript_api = NULL;
}

void GDN_EXPORT godot_nativescript_init(void *p_handle) {
    godot_instance_create_func create = { NULL, NULL, NULL };
    create.create_func = &sunvox_constructor;

    godot_instance_destroy_func destroy = { NULL, NULL, NULL };
    destroy.destroy_func = &sunvox_destructor;

    nativescript_api->godot_nativescript_register_class(p_handle, "Sunvox", "Reference",
            create, destroy);

    godot_instance_method init = { NULL, NULL, NULL };
    init.method = &sunvox_init;

    godot_instance_method load = { NULL, NULL, NULL };
    load.method = &sunvox_load;

    godot_instance_method load_from_memory = { NULL, NULL, NULL };
    load_from_memory.method = &sunvox_load_from_memory;

    godot_instance_method send_event = { NULL, NULL, NULL };
    send_event.method = &sunvox_send_event;

    godot_instance_method play_from_beginning = { NULL, NULL, NULL };
    play_from_beginning.method = &sunvox_play_from_beginning;

    godot_instance_method volume = { NULL, NULL, NULL };
    volume.method = &sunvox_volume;

    godot_method_attributes attributes = { GODOT_METHOD_RPC_MODE_DISABLED };

    nativescript_api->godot_nativescript_register_method(p_handle, "Sunvox", "init", attributes, init);
    nativescript_api->godot_nativescript_register_method(p_handle, "Sunvox", "load", attributes, load);
    nativescript_api->godot_nativescript_register_method(p_handle, "Sunvox", "load_from_memory", attributes, load_from_memory);
    nativescript_api->godot_nativescript_register_method(p_handle, "Sunvox", "send_event", attributes, send_event);
    nativescript_api->godot_nativescript_register_method(p_handle, "Sunvox", "play_from_beginning", attributes, play_from_beginning);
    nativescript_api->godot_nativescript_register_method(p_handle, "Sunvox", "volume", attributes, volume);
}

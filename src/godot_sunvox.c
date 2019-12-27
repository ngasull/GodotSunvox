#include <dlfcn.h>
#include <stdlib.h>

#include <gdnative_api_struct.gen.h>

#define SUNVOX_MAIN
#include "sunvox.h"

#define MAX_SLOTS 46

typedef struct user_data_struct {
  bool init;
  bool used_slot[MAX_SLOTS];
} user_data_struct;

const godot_gdnative_core_api_struct *api = NULL;
const godot_gdnative_ext_nativescript_api_struct *nativescript_api = NULL;

GDCALLINGCONV void *sunvox_constructor(godot_object *p_instance, void *p_method_data) {
  user_data_struct *user_data = (user_data_struct *)api->godot_alloc(sizeof(user_data_struct));
  user_data->init = false;

  for (int i = 0; i < MAX_SLOTS; i++) {
    user_data->used_slot[i] = false;
  }

  if (sv_load_dll()) {
    api->godot_print_error("sv_load_dll() failed", __func__, __FILE__, __LINE__);
  }

  return user_data;
}

GDCALLINGCONV void sunvox_destructor(godot_object *p_instance, void *p_method_data, void *p_user_data) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;

  for (int i = 0; i < MAX_SLOTS; i++) {
    if (user_data->used_slot[i]) {
      sv_stop(i);
      sv_close_slot(i);
    }
  }

  if (user_data->init) {
    sv_deinit();
  }

  sv_unload_dll();

  api->godot_free(p_user_data);
}

godot_variant sunvox_acquire_slot(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = -1;
  for (int i = 0; i < MAX_SLOTS && slot < 0; i++) {
    if (!user_data->used_slot[i]) {
      slot = i;
      sv_open_slot(slot);
      user_data->used_slot[slot] = true;
    }
  }
  api->godot_variant_new_int(&ret, slot);
  return ret;
}

godot_variant sunvox_close_slot(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int res = sv_close_slot(slot);
  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_connect_module(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int source = api->godot_variant_as_int(p_args[1]);
  int destination = api->godot_variant_as_int(p_args[2]);
  int res = sv_connect_module(slot, source, destination);
  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_disconnect_module(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int source = api->godot_variant_as_int(p_args[1]);
  int destination = api->godot_variant_as_int(p_args[2]);
  int res = sv_disconnect_module(slot, source, destination);
  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_end_of_song(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);

  int res = sv_end_of_song(slot);

  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_get_current_line(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);

  int res = sv_get_current_line(slot);

  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_get_current_line_real(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);

  int res = sv_get_current_line2(slot);
  double real = (double)(res >> 5) + ((double)(0x1F & res) / 0x20);

  api->godot_variant_new_real(&ret, real);
  return ret;
}

godot_variant sunvox_get_module_name(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int i = api->godot_variant_as_int(p_args[1]);
  godot_string name_str = api->godot_string_chars_to_utf8(sv_get_module_name(slot, i));
  api->godot_variant_new_string(&ret, &name_str);
  api->godot_string_destroy(&name_str);
  return ret;
}

godot_variant sunvox_get_song_bpm(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  int slot = api->godot_variant_as_int(p_args[0]);
  godot_variant ret;

  int res = sv_get_song_bpm(slot);

  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_get_song_length_frames(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  int slot = api->godot_variant_as_int(p_args[0]);
  godot_variant ret;

  unsigned int res = sv_get_song_length_frames(slot);

  api->godot_variant_new_uint(&ret, res);
  return ret;
}

godot_variant sunvox_get_song_length_lines(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  int slot = api->godot_variant_as_int(p_args[0]);
  godot_variant ret;

  unsigned int res = sv_get_song_length_lines(slot);

  api->godot_variant_new_uint(&ret, res);
  return ret;
}

godot_variant sunvox_get_song_name(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  godot_string name_str = api->godot_string_chars_to_utf8(sv_get_song_name(slot));
  api->godot_variant_new_string(&ret, &name_str);
  api->godot_string_destroy(&name_str);
  return ret;
}

godot_variant sunvox_get_song_tpl(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);

  int res = sv_get_song_tpl(slot);

  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_get_ticks(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;

  unsigned int res = sv_get_ticks();

  api->godot_variant_new_uint(&ret, res);
  return ret;
}

godot_variant sunvox_init(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  godot_string config = api->godot_variant_as_string(p_args[0]);
  godot_char_string config_str = api->godot_string_ascii(&config);
  int frequency = api->godot_variant_as_int(p_args[1]);
  int channels = api->godot_variant_as_int(p_args[2]);
  int flags = api->godot_variant_as_int(p_args[3]);

  if (user_data->init) {
    sv_deinit();
  }

  int ver = sv_init(api->godot_char_string_get_data(&config_str), frequency, channels, flags);
  bool success = ver >= 0;
  if (success) {
  } else {
    api->godot_print_error("sv_init() error", __func__, __FILE__, __LINE__);
  }

  user_data->init = success;

  api->godot_char_string_destroy(&config_str);
  api->godot_string_destroy(&config);
  api->godot_variant_new_bool(&ret, success);
  return ret;
}

godot_variant sunvox_load(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  godot_string file_name = api->godot_variant_as_string(p_args[1]);
  godot_char_string char_str = api->godot_string_ascii(&file_name);

  int res = sv_load(slot, api->godot_char_string_get_data(&char_str));

  api->godot_variant_new_int(&ret, res);
  api->godot_char_string_destroy(&char_str);
  api->godot_string_destroy(&file_name);
  return ret;
}

godot_variant sunvox_load_from_memory(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  godot_pool_byte_array data = api->godot_variant_as_pool_byte_array(p_args[1]);

  godot_int size = api->godot_pool_byte_array_size(&data);
  godot_pool_byte_array_read_access *p_read = api->godot_pool_byte_array_read(&data);
  int res = sv_load_from_memory(slot, (void *)api->godot_pool_byte_array_read_access_ptr(p_read), size);

  api->godot_variant_new_int(&ret, res);
  api->godot_pool_byte_array_read_access_destroy(p_read);
  api->godot_pool_byte_array_destroy(&data);
  return ret;
}

godot_variant sunvox_load_module(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  godot_string file_name = api->godot_variant_as_string(p_args[1]);
  godot_char_string char_str = api->godot_string_ascii(&file_name);
  int x = api->godot_variant_as_int(p_args[2]);
  int y = api->godot_variant_as_int(p_args[3]);
  int z = api->godot_variant_as_int(p_args[4]);

  int res = sv_load_module(slot, api->godot_char_string_get_data(&char_str), x, y, z);

  api->godot_variant_new_int(&ret, res);
  api->godot_char_string_destroy(&char_str);
  api->godot_string_destroy(&file_name);
  return ret;
}

godot_variant sunvox_load_module_from_memory(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  godot_pool_byte_array data = api->godot_variant_as_pool_byte_array(p_args[1]);
  int x = api->godot_variant_as_int(p_args[2]);
  int y = api->godot_variant_as_int(p_args[3]);
  int z = api->godot_variant_as_int(p_args[4]);

  godot_int size = api->godot_pool_byte_array_size(&data);
  godot_pool_byte_array_read_access *p_read = api->godot_pool_byte_array_read(&data);
  int res = sv_load_module_from_memory(slot, (void *)api->godot_pool_byte_array_read_access_ptr(p_read), size, x, y, z);

  api->godot_variant_new_int(&ret, res);
  api->godot_pool_byte_array_read_access_destroy(p_read);
  api->godot_pool_byte_array_destroy(&data);
  return ret;
}

godot_variant sunvox_lock_slot(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);

  int res = sv_lock_slot(slot);

  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_open_slot(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int res = sv_open_slot(slot);
  user_data->used_slot[slot] = true;
  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_play(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);

  int res = sv_play(slot);

  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_play_from_beginning(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);

  int res = sv_play_from_beginning(slot);

  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_rewind(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int line_number = api->godot_variant_as_int(p_args[1]);

  int res = sv_rewind(slot, line_number);

  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_send_event(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int track_num = api->godot_variant_as_int(p_args[1]);
  int note = api->godot_variant_as_int(p_args[2]);
  int vel = api->godot_variant_as_int(p_args[3]);
  int module = api->godot_variant_as_int(p_args[4]);
  int ctl = api->godot_variant_as_int(p_args[5]);
  int ctl_val = api->godot_variant_as_int(p_args[6]);

  int res = sv_send_event(slot, track_num, note, vel, module + 1, ctl, ctl_val);

  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_set_autostop(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int autostop = api->godot_variant_as_int(p_args[1]);

  int res = sv_set_autostop(slot, autostop);

  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_stop(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);

  int res = sv_stop(slot);

  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_unlock_slot(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);

  int res = sv_unlock_slot(slot);

  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_volume(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int vol = api->godot_variant_as_int(p_args[1]);

  int res = sv_volume(slot, vol);

  api->godot_variant_new_int(&ret, res);
  return ret;
}

void godot_sunvox_register_method(void *p_handle, const char *name, godot_method_attributes p_attr, void *method) {
  godot_instance_method instance_method = {NULL, NULL, NULL};
  instance_method.method = method;
  nativescript_api->godot_nativescript_register_method(p_handle, "Sunvox", name, p_attr, instance_method);
}

void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *p_options) {
  api = p_options->api_struct;

  // Now find our extensions.
  for (int i = 0; i < api->num_extensions; i++) {
    switch (api->extensions[i]->type) {
    case GDNATIVE_EXT_NATIVESCRIPT: {
      nativescript_api = (godot_gdnative_ext_nativescript_api_struct *)api->extensions[i];
    }; break;
    default:
      break;
    }
  }
}

void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *p_options) {
  api = NULL;
  nativescript_api = NULL;
}

void GDN_EXPORT godot_nativescript_init(void *p_handle) {
  godot_instance_create_func create = {NULL, NULL, NULL};
  create.create_func = &sunvox_constructor;

  godot_instance_destroy_func destroy = {NULL, NULL, NULL};
  destroy.destroy_func = &sunvox_destructor;

  nativescript_api->godot_nativescript_register_class(p_handle, "Sunvox", "Node", create, destroy);

  godot_method_attributes attributes = {GODOT_METHOD_RPC_MODE_DISABLED};

  godot_sunvox_register_method(p_handle, "acquire_slot", attributes, &sunvox_acquire_slot);
  godot_sunvox_register_method(p_handle, "close_slot", attributes, &sunvox_close_slot);
  godot_sunvox_register_method(p_handle, "connect_module", attributes, &sunvox_connect_module);
  godot_sunvox_register_method(p_handle, "disconnect_module", attributes, &sunvox_disconnect_module);
  godot_sunvox_register_method(p_handle, "end_of_song", attributes, &sunvox_end_of_song);
  godot_sunvox_register_method(p_handle, "get_current_line", attributes, &sunvox_get_current_line);
  godot_sunvox_register_method(p_handle, "get_current_line_real", attributes, &sunvox_get_current_line_real);
  godot_sunvox_register_method(p_handle, "get_module_name", attributes, &sunvox_get_module_name);
  godot_sunvox_register_method(p_handle, "get_song_bpm", attributes, &sunvox_get_song_bpm);
  godot_sunvox_register_method(p_handle, "get_song_length_frames", attributes, &sunvox_get_song_length_frames);
  godot_sunvox_register_method(p_handle, "get_song_name", attributes, &sunvox_get_song_name);
  godot_sunvox_register_method(p_handle, "get_song_tpl", attributes, &sunvox_get_song_tpl);
  godot_sunvox_register_method(p_handle, "get_ticks", attributes, &sunvox_get_ticks);
  godot_sunvox_register_method(p_handle, "init", attributes, &sunvox_init);
  godot_sunvox_register_method(p_handle, "load", attributes, &sunvox_load);
  godot_sunvox_register_method(p_handle, "load_from_memory", attributes, &sunvox_load_from_memory);
  godot_sunvox_register_method(p_handle, "load_module", attributes, &sunvox_load_module);
  godot_sunvox_register_method(p_handle, "load_module_from_memory", attributes, &sunvox_load_module_from_memory);
  godot_sunvox_register_method(p_handle, "lock_slot", attributes, &sunvox_lock_slot);
  godot_sunvox_register_method(p_handle, "open_slot", attributes, &sunvox_open_slot);
  godot_sunvox_register_method(p_handle, "play", attributes, &sunvox_play);
  godot_sunvox_register_method(p_handle, "play_from_beginning", attributes, &sunvox_play_from_beginning);
  godot_sunvox_register_method(p_handle, "rewind", attributes, &sunvox_rewind);
  godot_sunvox_register_method(p_handle, "send_event", attributes, &sunvox_send_event);
  godot_sunvox_register_method(p_handle, "set_autostop", attributes, &sunvox_set_autostop);
  godot_sunvox_register_method(p_handle, "stop", attributes, &sunvox_stop);
  godot_sunvox_register_method(p_handle, "unlock_slot", attributes, &sunvox_unlock_slot);
  godot_sunvox_register_method(p_handle, "volume", attributes, &sunvox_volume);
}

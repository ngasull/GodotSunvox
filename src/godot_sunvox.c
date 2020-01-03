#include <dlfcn.h>
#include <stdlib.h>

#include <gdnative_api_struct.gen.h>

#define SUNVOX_MAIN
#include "sunvox.h"

typedef struct user_data_struct {
  bool init;
  int used_slot;
} user_data_struct;

const godot_gdnative_core_api_struct *api = NULL;
const godot_gdnative_ext_nativescript_api_struct *nativescript_api = NULL;

GDCALLINGCONV void *sunvox_constructor(godot_object *p_instance, void *p_method_data) {
  user_data_struct *user_data = (user_data_struct *)api->godot_alloc(sizeof(user_data_struct));
  user_data->init = false;
  user_data->used_slot = 0;

  if (sv_load_dll()) {
    // FIXME bug in Sunvox beta where it correctly loads DLL but errors
    // api->godot_print_error("sv_load_dll() failed", __func__, __FILE__, __LINE__);
  }

  return user_data;
}

GDCALLINGCONV void sunvox_destructor(godot_object *p_instance, void *p_method_data, void *p_user_data) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;

  for (int i = 0; i < 16; i++) {
    if (user_data->used_slot & (1 << i)) {
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
  for (int i = 0; i < 16 && slot < 0; i++) {
    if (!(user_data->used_slot & (1 << i))) {
      slot = i;
      sv_open_slot(slot);
      user_data->used_slot |= (1 << slot);
    }
  }
  api->godot_variant_new_int(&ret, slot);
  return ret;
}

godot_variant sunvox_audio_callback(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int frames = api->godot_variant_as_int(p_args[0]);
  int latency = api->godot_variant_as_int(p_args[1]);
  uint32_t out_time = api->godot_variant_as_uint(p_args[2]);

  int n_bytes = 2 * 2 * frames; // 1 frame = 2 channels (stereo) x (2 x 8 = 16) bits
  godot_pool_byte_array frames_bytearray;
  api->godot_pool_byte_array_new(&frames_bytearray);
  api->godot_pool_byte_array_resize(&frames_bytearray, n_bytes);

  uint8_t *buf = malloc(n_bytes);
  int res = sv_audio_callback(buf, frames, latency, out_time);

  for (int i = 0; i < n_bytes; i++) {
    api->godot_pool_byte_array_set(&frames_bytearray, i, buf[i]);
  }
  free(buf);

  api->godot_variant_new_pool_byte_array(&ret, &frames_bytearray);
  api->godot_pool_byte_array_destroy(&frames_bytearray);
  return ret;
}

godot_variant sunvox_audio_callback2(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int frames = api->godot_variant_as_int(p_args[0]);
  int latency = api->godot_variant_as_int(p_args[1]);
  uint32_t out_time = api->godot_variant_as_uint(p_args[2]);
  int in_type = api->godot_variant_as_int(p_args[3]);
  int in_channels = api->godot_variant_as_int(p_args[4]);
  godot_pool_byte_array in_buf = api->godot_variant_as_pool_byte_array(p_args[5]);
  godot_pool_byte_array_read_access *p_read = api->godot_pool_byte_array_read(&in_buf);

  int n_bytes = 2 * 2 * frames; // 1 frame = 2 channels (stereo) x (2 x 8 = 16) bits
  godot_pool_byte_array frames_bytearray;
  api->godot_pool_byte_array_new(&frames_bytearray);
  api->godot_pool_byte_array_resize(&frames_bytearray, n_bytes);

  uint8_t *buf = malloc(n_bytes);
  int res = sv_audio_callback2(buf, frames, latency, out_time, in_type, in_channels, (void *)api->godot_pool_byte_array_read_access_ptr(p_read));

  for (int i = 0; i < n_bytes; i++) {
    api->godot_pool_byte_array_set(&frames_bytearray, i, buf[i]);
  }
  free(buf);

  api->godot_pool_byte_array_read_access_destroy(p_read);
  api->godot_pool_byte_array_destroy(&in_buf);
  api->godot_variant_new_pool_byte_array(&ret, &frames_bytearray);
  api->godot_pool_byte_array_destroy(&frames_bytearray);
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

godot_variant sunvox_deinit(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int res = sv_deinit();
  user_data->init = false;
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

godot_variant sunvox_get_current_line2(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);

  int res = sv_get_current_line2(slot);
  double real = (double)(res >> 5) + ((double)(0x1F & res) / 0x20);

  api->godot_variant_new_real(&ret, real);
  return ret;
}

godot_variant sunvox_get_current_signal_level(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int channel = api->godot_variant_as_int(p_args[1]);
  int res = sv_get_current_signal_level(slot, channel);
  api->godot_variant_new_int(&ret, res);
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

godot_variant sunvox_get_log(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int size = api->godot_variant_as_int(p_args[0]);
  godot_string log_str = api->godot_string_chars_to_utf8(sv_get_log(size));
  api->godot_variant_new_string(&ret, &log_str);
  api->godot_string_destroy(&log_str);
  return ret;
}

godot_variant sunvox_get_module_color(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int mod_num = api->godot_variant_as_int(p_args[1]);
  int res = sv_get_module_color(slot, mod_num);
  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_get_module_ctl_name(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int mod_num = api->godot_variant_as_int(p_args[1]);
  int ctl_num = api->godot_variant_as_int(p_args[2]);
  godot_string name_str = api->godot_string_chars_to_utf8(sv_get_module_ctl_name(slot, mod_num, ctl_num));
  api->godot_variant_new_string(&ret, &name_str);
  api->godot_string_destroy(&name_str);
  return ret;
}

godot_variant sunvox_get_module_ctl_value(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int mod_num = api->godot_variant_as_int(p_args[1]);
  int ctl_num = api->godot_variant_as_int(p_args[2]);
  int scaled = api->godot_variant_as_int(p_args[3]);
  int res = sv_get_module_ctl_value(slot, mod_num, ctl_num, scaled);
  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_get_module_flags(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int mod_num = api->godot_variant_as_int(p_args[1]);
  uint32_t res = sv_get_module_flags(slot, mod_num);
  api->godot_variant_new_uint(&ret, res);
  return ret;
}

godot_variant sunvox_get_module_inputs(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int mod_num = api->godot_variant_as_int(p_args[1]);

  int *inputs = sv_get_module_inputs(slot, mod_num);
  int n_inputs = (sv_get_module_flags(slot, mod_num) & SV_MODULE_INPUTS_MASK) >> SV_MODULE_INPUTS_OFF;
  godot_pool_int_array inputs_intarray;
  api->godot_pool_int_array_new(&inputs_intarray);

  for (int i = 0; i < n_inputs; i++) {
    if (inputs[i] > -1) {
      api->godot_pool_int_array_append(&inputs_intarray, inputs[i]);
    }
  }

  api->godot_variant_new_pool_int_array(&ret, &inputs_intarray);
  api->godot_pool_int_array_destroy(&inputs_intarray);
  return ret;
}

godot_variant sunvox_get_module_outputs(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int mod_num = api->godot_variant_as_int(p_args[1]);

  int *outputs = sv_get_module_outputs(slot, mod_num);
  int n_outputs = (sv_get_module_flags(slot, mod_num) & SV_MODULE_OUTPUTS_MASK) >> SV_MODULE_OUTPUTS_OFF;
  godot_pool_int_array outputs_intarray;
  api->godot_pool_int_array_new(&outputs_intarray);

  for (int i = 0; i < n_outputs; i++) {
    if (outputs[i] > -1) {
      api->godot_pool_int_array_append(&outputs_intarray, outputs[i]);
    }
  }

  api->godot_variant_new_pool_int_array(&ret, &outputs_intarray);
  api->godot_pool_int_array_destroy(&outputs_intarray);
  return ret;
}

godot_variant sunvox_get_module_scope2(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int mod_num = api->godot_variant_as_int(p_args[1]);
  int channel = api->godot_variant_as_int(p_args[2]);
  uint32_t samples_to_read = api->godot_variant_as_uint(p_args[3]);

  int16_t *dest_buf = malloc(samples_to_read * sizeof(int16_t));
  uint32_t res = sv_get_module_scope2(slot, mod_num, channel, dest_buf, samples_to_read);
  godot_pool_int_array dest_intarray;
  api->godot_pool_int_array_new(&dest_intarray);
  api->godot_pool_int_array_resize(&dest_intarray, res);

  for (int i = 0; i < res; i++) {
    api->godot_pool_int_array_set(&dest_intarray, i, dest_buf[i]);
  }

  free(dest_buf);
  api->godot_variant_new_pool_int_array(&ret, &dest_intarray);
  api->godot_pool_int_array_destroy(&dest_intarray);
  return ret;
}

godot_variant sunvox_get_module_xy(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int mod_num = api->godot_variant_as_int(p_args[1]);

  int x;
  int y;
  SV_GET_MODULE_XY(sv_get_module_xy(slot, mod_num), x, y);
  godot_pool_int_array xy_array;
  api->godot_pool_int_array_new(&xy_array);
  api->godot_pool_int_array_resize(&xy_array, 2);
  api->godot_pool_int_array_set(&xy_array, 0, x);
  api->godot_pool_int_array_set(&xy_array, 1, y);

  api->godot_variant_new_pool_int_array(&ret, &xy_array);
  api->godot_pool_int_array_destroy(&xy_array);
  return ret;
}

godot_variant sunvox_get_number_of_modules(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int res = sv_get_number_of_modules(slot);
  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_get_number_of_module_ctls(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int mod_num = api->godot_variant_as_int(p_args[1]);
  int res = sv_get_number_of_module_ctls(slot, mod_num);
  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_get_number_of_patterns(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int res = sv_get_number_of_patterns(slot);
  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_get_pattern_data(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int pat_num = api->godot_variant_as_int(p_args[1]);
  sunvox_note *notes = sv_get_pattern_data(slot, pat_num);

  godot_array tracks;
  godot_array lines;
  godot_variant lines_variant;
  godot_dictionary line;
  godot_variant line_variant;
  godot_string key_str;
  godot_variant key;
  godot_variant value;

  int n_lines = sv_get_pattern_lines(slot, pat_num);
  int n_tracks = sv_get_pattern_tracks(slot, pat_num);

  api->godot_array_new(&tracks);
  api->godot_array_resize(&tracks, n_tracks);

  for (int t = 0; t < n_tracks; t++) {
    api->godot_array_new(&lines);
    api->godot_array_resize(&lines, n_lines);

    for (int l = 0; l < n_lines; l++) {
      sunvox_note *note = &notes[l * n_tracks + t];
      api->godot_dictionary_new(&line);

      key_str = api->godot_string_chars_to_utf8("note");
      api->godot_variant_new_string(&key, &key_str);
      api->godot_variant_new_int(&value, note->note);
      api->godot_string_destroy(&key_str);
      api->godot_dictionary_set(&line, &key, &value);
      api->godot_variant_destroy(&key);
      api->godot_variant_destroy(&value);

      key_str = api->godot_string_chars_to_utf8("vel");
      api->godot_variant_new_string(&key, &key_str);
      api->godot_variant_new_int(&value, note->vel);
      api->godot_string_destroy(&key_str);
      api->godot_dictionary_set(&line, &key, &value);
      api->godot_variant_destroy(&key);
      api->godot_variant_destroy(&value);

      key_str = api->godot_string_chars_to_utf8("module");
      api->godot_variant_new_string(&key, &key_str);
      api->godot_variant_new_int(&value, note->module - 1);
      api->godot_string_destroy(&key_str);
      api->godot_dictionary_set(&line, &key, &value);
      api->godot_variant_destroy(&key);
      api->godot_variant_destroy(&value);

      key_str = api->godot_string_chars_to_utf8("ctl");
      api->godot_variant_new_string(&key, &key_str);
      api->godot_variant_new_int(&value, (note->ctl & 0xFF00) >> 8);
      api->godot_string_destroy(&key_str);
      api->godot_dictionary_set(&line, &key, &value);
      api->godot_variant_destroy(&key);
      api->godot_variant_destroy(&value);

      key_str = api->godot_string_chars_to_utf8("effect");
      api->godot_variant_new_string(&key, &key_str);
      api->godot_variant_new_int(&value, note->ctl & 0xFF);
      api->godot_string_destroy(&key_str);
      api->godot_dictionary_set(&line, &key, &value);
      api->godot_variant_destroy(&key);
      api->godot_variant_destroy(&value);

      key_str = api->godot_string_chars_to_utf8("ctl_val");
      api->godot_variant_new_string(&key, &key_str);
      api->godot_variant_new_int(&value, note->ctl_val);
      api->godot_string_destroy(&key_str);
      api->godot_dictionary_set(&line, &key, &value);
      api->godot_variant_destroy(&key);
      api->godot_variant_destroy(&value);

      api->godot_variant_new_dictionary(&line_variant, &line);
      api->godot_array_set(&lines, l, &line_variant);
      api->godot_dictionary_destroy(&line);
    }

    api->godot_variant_new_array(&lines_variant, &lines);
    api->godot_array_set(&tracks, t, &lines_variant);
    api->godot_array_destroy(&lines);
  }

  api->godot_variant_new_array(&ret, &tracks);
  api->godot_array_destroy(&tracks);
  return ret;
}

godot_variant sunvox_get_pattern_lines(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int pat_num = api->godot_variant_as_int(p_args[1]);
  int res = sv_get_pattern_lines(slot, pat_num);
  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_get_pattern_tracks(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int pat_num = api->godot_variant_as_int(p_args[1]);
  int res = sv_get_pattern_tracks(slot, pat_num);
  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_get_pattern_x(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int pat_num = api->godot_variant_as_int(p_args[1]);
  int res = sv_get_pattern_x(slot, pat_num);
  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_get_pattern_y(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int pat_num = api->godot_variant_as_int(p_args[1]);
  int res = sv_get_pattern_y(slot, pat_num);
  api->godot_variant_new_int(&ret, res);
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
  uint32_t res = sv_get_song_length_frames(slot);
  api->godot_variant_new_uint(&ret, res);
  return ret;
}

godot_variant sunvox_get_song_length_lines(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  int slot = api->godot_variant_as_int(p_args[0]);
  godot_variant ret;
  uint32_t res = sv_get_song_length_lines(slot);
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
  uint32_t res = sv_get_ticks();
  api->godot_variant_new_uint(&ret, res);
  return ret;
}

godot_variant sunvox_get_ticks_per_second(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  uint32_t res = sv_get_ticks_per_second();
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
  uint32_t flags = api->godot_variant_as_int(p_args[3]);

  if (user_data->init) {
    sv_deinit();
  }

  int ver = sv_init(api->godot_char_string_get_data(&config_str), frequency, channels, flags);
  bool success = ver >= 0;
  if (!success) {
    api->godot_print_error("sv_init() error", __func__, __FILE__, __LINE__);
  }

  user_data->init = success;

  api->godot_char_string_destroy(&config_str);
  api->godot_string_destroy(&config);
  api->godot_variant_new_bool(&ret, success);
  return ret;
}

godot_variant sunvox_load(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
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
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  godot_pool_byte_array data = api->godot_variant_as_pool_byte_array(p_args[1]);

  uint32_t size = api->godot_pool_byte_array_size(&data);
  godot_pool_byte_array_read_access *p_read = api->godot_pool_byte_array_read(&data);
  int res = sv_load_from_memory(slot, (void *)api->godot_pool_byte_array_read_access_ptr(p_read), size);

  api->godot_variant_new_int(&ret, res);
  api->godot_pool_byte_array_read_access_destroy(p_read);
  api->godot_pool_byte_array_destroy(&data);
  return ret;
}

godot_variant sunvox_load_module(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
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
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  godot_pool_byte_array data = api->godot_variant_as_pool_byte_array(p_args[1]);
  int x = api->godot_variant_as_int(p_args[2]);
  int y = api->godot_variant_as_int(p_args[3]);
  int z = api->godot_variant_as_int(p_args[4]);

  uint32_t size = api->godot_pool_byte_array_size(&data);
  godot_pool_byte_array_read_access *p_read = api->godot_pool_byte_array_read(&data);
  int res = sv_load_module_from_memory(slot, (void *)api->godot_pool_byte_array_read_access_ptr(p_read), size, x, y, z);

  api->godot_variant_new_int(&ret, res);
  api->godot_pool_byte_array_read_access_destroy(p_read);
  api->godot_pool_byte_array_destroy(&data);
  return ret;
}

godot_variant sunvox_lock_slot(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int res = sv_lock_slot(slot);
  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_new_module(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  godot_string type = api->godot_variant_as_string(p_args[1]);
  godot_char_string type_str = api->godot_string_ascii(&type);
  godot_string name = api->godot_variant_as_string(p_args[2]);
  godot_char_string name_str = api->godot_string_ascii(&name);
  int x = api->godot_variant_as_int(p_args[3]);
  int y = api->godot_variant_as_int(p_args[4]);
  int z = api->godot_variant_as_int(p_args[5]);

  int res = sv_new_module(slot, api->godot_char_string_get_data(&type_str), api->godot_char_string_get_data(&name_str), x, y, z);

  api->godot_variant_new_int(&ret, res);
  api->godot_char_string_destroy(&type_str);
  api->godot_string_destroy(&type);
  api->godot_char_string_destroy(&name_str);
  api->godot_string_destroy(&name);
  return ret;
}

godot_variant sunvox_open_slot(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  user_data_struct *user_data = (user_data_struct *)p_user_data;
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int res = sv_open_slot(slot);
  user_data->used_slot |= (1 << slot);
  api->godot_variant_new_int(&ret, res);
  return ret;
}

godot_variant sunvox_pattern_mute(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int pat_num = api->godot_variant_as_int(p_args[1]);
  int mute = api->godot_variant_as_int(p_args[2]);
  int res = sv_pattern_mute(slot, pat_num, mute);
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

godot_variant sunvox_remove_module(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int mod_num = api->godot_variant_as_int(p_args[1]);
  int res = sv_remove_module(slot, mod_num);
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

godot_variant sunvox_sampler_load(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int sampler_module = api->godot_variant_as_int(p_args[1]);
  godot_string file_name = api->godot_variant_as_string(p_args[2]);
  int sample_slot = api->godot_variant_as_int(p_args[3]);
  godot_char_string char_str = api->godot_string_ascii(&file_name);

  int res = sv_sampler_load(slot, sampler_module, api->godot_char_string_get_data(&char_str), sample_slot);

  api->godot_variant_new_int(&ret, res);
  api->godot_char_string_destroy(&char_str);
  api->godot_string_destroy(&file_name);
  return ret;
}

godot_variant sunvox_sampler_load_from_memory(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int slot = api->godot_variant_as_int(p_args[0]);
  int sampler_module = api->godot_variant_as_int(p_args[1]);
  godot_pool_byte_array data = api->godot_variant_as_pool_byte_array(p_args[2]);
  int sample_slot = api->godot_variant_as_int(p_args[3]);

  uint32_t size = api->godot_pool_byte_array_size(&data);
  godot_pool_byte_array_read_access *p_read = api->godot_pool_byte_array_read(&data);
  int res = sv_sampler_load_from_memory(slot, sampler_module, (void *)api->godot_pool_byte_array_read_access_ptr(p_read), size, sample_slot);

  api->godot_variant_new_int(&ret, res);
  api->godot_pool_byte_array_read_access_destroy(p_read);
  api->godot_pool_byte_array_destroy(&data);
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
  int effect = api->godot_variant_as_int(p_args[6]);
  int ctl_val = api->godot_variant_as_int(p_args[7]);

  int res = sv_send_event(slot, track_num, note, vel, module + 1, ((ctl & 0xFF) << 8) | (effect & 0xFF), ctl_val);

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

godot_variant sunvox_update_input(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
  godot_variant ret;
  int res = sv_update_input();
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
  godot_sunvox_register_method(p_handle, "audio_callback", attributes, &sunvox_audio_callback);
  godot_sunvox_register_method(p_handle, "audio_callback2", attributes, &sunvox_audio_callback2);
  godot_sunvox_register_method(p_handle, "close_slot", attributes, &sunvox_close_slot);
  godot_sunvox_register_method(p_handle, "connect_module", attributes, &sunvox_connect_module);
  godot_sunvox_register_method(p_handle, "deinit", attributes, &sunvox_deinit);
  godot_sunvox_register_method(p_handle, "disconnect_module", attributes, &sunvox_disconnect_module);
  godot_sunvox_register_method(p_handle, "end_of_song", attributes, &sunvox_end_of_song);
  godot_sunvox_register_method(p_handle, "get_current_line", attributes, &sunvox_get_current_line);
  godot_sunvox_register_method(p_handle, "get_current_line2", attributes, &sunvox_get_current_line2);
  godot_sunvox_register_method(p_handle, "get_current_signal_level", attributes, &sunvox_get_current_signal_level);
  godot_sunvox_register_method(p_handle, "get_log", attributes, &sunvox_get_log);
  godot_sunvox_register_method(p_handle, "get_module_color", attributes, &sunvox_get_module_color);
  godot_sunvox_register_method(p_handle, "get_module_ctl_name", attributes, &sunvox_get_module_ctl_name);
  godot_sunvox_register_method(p_handle, "get_module_ctl_value", attributes, &sunvox_get_module_ctl_value);
  godot_sunvox_register_method(p_handle, "get_module_flags", attributes, &sunvox_get_module_flags);
  godot_sunvox_register_method(p_handle, "get_module_inputs", attributes, &sunvox_get_module_inputs);
  godot_sunvox_register_method(p_handle, "get_module_name", attributes, &sunvox_get_module_name);
  godot_sunvox_register_method(p_handle, "get_module_outputs", attributes, &sunvox_get_module_outputs);
  godot_sunvox_register_method(p_handle, "get_module_scope2", attributes, &sunvox_get_module_scope2);
  godot_sunvox_register_method(p_handle, "get_module_xy", attributes, &sunvox_get_module_xy);
  godot_sunvox_register_method(p_handle, "get_number_of_modules", attributes, &sunvox_get_number_of_modules);
  godot_sunvox_register_method(p_handle, "get_number_of_module_ctls", attributes, &sunvox_get_number_of_module_ctls);
  godot_sunvox_register_method(p_handle, "get_number_of_patterns", attributes, &sunvox_get_number_of_patterns);
  godot_sunvox_register_method(p_handle, "get_pattern_data", attributes, &sunvox_get_pattern_data);
  godot_sunvox_register_method(p_handle, "get_pattern_lines", attributes, &sunvox_get_pattern_lines);
  godot_sunvox_register_method(p_handle, "get_pattern_tracks", attributes, &sunvox_get_pattern_tracks);
  godot_sunvox_register_method(p_handle, "get_pattern_x", attributes, &sunvox_get_pattern_x);
  godot_sunvox_register_method(p_handle, "get_pattern_y", attributes, &sunvox_get_pattern_y);
  godot_sunvox_register_method(p_handle, "get_song_bpm", attributes, &sunvox_get_song_bpm);
  godot_sunvox_register_method(p_handle, "get_song_length_frames", attributes, &sunvox_get_song_length_frames);
  godot_sunvox_register_method(p_handle, "get_song_length_lines", attributes, &sunvox_get_song_length_lines);
  godot_sunvox_register_method(p_handle, "get_song_name", attributes, &sunvox_get_song_name);
  godot_sunvox_register_method(p_handle, "get_song_tpl", attributes, &sunvox_get_song_tpl);
  godot_sunvox_register_method(p_handle, "get_ticks", attributes, &sunvox_get_ticks);
  godot_sunvox_register_method(p_handle, "get_ticks_per_second", attributes, &sunvox_get_ticks_per_second);
  godot_sunvox_register_method(p_handle, "init", attributes, &sunvox_init);
  godot_sunvox_register_method(p_handle, "load", attributes, &sunvox_load);
  godot_sunvox_register_method(p_handle, "load_from_memory", attributes, &sunvox_load_from_memory);
  godot_sunvox_register_method(p_handle, "load_module", attributes, &sunvox_load_module);
  godot_sunvox_register_method(p_handle, "load_module_from_memory", attributes, &sunvox_load_module_from_memory);
  godot_sunvox_register_method(p_handle, "lock_slot", attributes, &sunvox_lock_slot);
  godot_sunvox_register_method(p_handle, "new_module", attributes, &sunvox_new_module);
  godot_sunvox_register_method(p_handle, "open_slot", attributes, &sunvox_open_slot);
  godot_sunvox_register_method(p_handle, "pattern_mute", attributes, &sunvox_pattern_mute);
  godot_sunvox_register_method(p_handle, "play", attributes, &sunvox_play);
  godot_sunvox_register_method(p_handle, "play_from_beginning", attributes, &sunvox_play_from_beginning);
  godot_sunvox_register_method(p_handle, "remove_module", attributes, &sunvox_remove_module);
  godot_sunvox_register_method(p_handle, "rewind", attributes, &sunvox_rewind);
  godot_sunvox_register_method(p_handle, "sampler_load", attributes, &sunvox_sampler_load);
  godot_sunvox_register_method(p_handle, "sampler_load_from_memory", attributes, &sunvox_sampler_load_from_memory);
  godot_sunvox_register_method(p_handle, "send_event", attributes, &sunvox_send_event);
  godot_sunvox_register_method(p_handle, "set_autostop", attributes, &sunvox_set_autostop);
  godot_sunvox_register_method(p_handle, "stop", attributes, &sunvox_stop);
  godot_sunvox_register_method(p_handle, "unlock_slot", attributes, &sunvox_unlock_slot);
  godot_sunvox_register_method(p_handle, "update_input", attributes, &sunvox_update_input);
  godot_sunvox_register_method(p_handle, "volume", attributes, &sunvox_volume);
}

extends Node

var slot

const SV_INIT_FLAG_USER_AUDIO_CALLBACK = 1 << 1
const SV_INIT_FLAG_AUDIO_INT16 = 1 << 2

func _ready():
  Sunvox.init("", 44100, 2, 0)
  slot = Sunvox.acquire_slot()
  Sunvox.load_from_memory(slot, preload("res://sunvox_test.res").__data__)
  Sunvox.set_autostop(slot, 1)
  Sunvox.volume(slot, 160)

  print("Playing %s" % Sunvox.get_song_name(slot))
  print("BPM: %d" % Sunvox.get_song_bpm(slot))
  print("Ticks per line: %d" % Sunvox.get_song_tpl(slot))

  print("Number of patterns %d" % Sunvox.get_number_of_patterns(slot))
  var pattern = 0
  var d = Sunvox.get_pattern_data(slot, pattern)[5][3] # 4th note 6th track
  print("Pattern #%d note=%d vel=%d module=%d ctl=%d effect=%d ctl_val=%d" % [pattern, d.note, d.vel, d.module, d.ctl, d.effect, d.ctl_val])

  var m = 6
  print("Sending note to module #%d %s" % [m, Sunvox.get_module_name(slot, m)])
  print("Inputs of module #%d %s" % [m, Sunvox.get_module_inputs(slot, m)])
  print("Outputs of module #%d %s" % [m, Sunvox.get_module_outputs(slot, m)])
  print("XY of module #%d %s" % [m, Sunvox.get_module_xy(slot, m)])

  for c in Sunvox.get_number_of_module_ctls(slot, m):
    print("Module #%d %s=%d" % [m, Sunvox.get_module_ctl_name(slot, m, c), Sunvox.get_module_ctl_value(slot, m, c)])

  Sunvox.send_event(slot, 0, 64, 129, 6, 0, 0, 0) # slot 0, track 0; note 64; velocity 129 (max); module 6
  Sunvox.play_from_beginning(slot)

#  SunvoxCallback.init("", 44100, 2, SV_INIT_FLAG_USER_AUDIO_CALLBACK | SV_INIT_FLAG_AUDIO_INT16)
#
#  var stream = AudioStreamSample.new()
#  var user_out_time = 1000
#  var user_cur_time = 0
#  var user_ticks_per_second = 80
#  var user_latency = user_out_time - user_cur_time
#  var sunvox_latency = (user_latency * SunvoxCallback.get_ticks_per_second()) / user_ticks_per_second
#  var latency_frames = (user_latency * 44100) / user_ticks_per_second
#  stream.format = AudioStreamSample.FORMAT_16_BITS
#  stream.stereo = true
#  stream.data = SunvoxCallback.audio_callback(44100, latency_frames, SunvoxCallback.get_ticks() + sunvox_latency)
#
#  var player = AudioStreamPlayer.new()
#  add_child(player)
#  player.stream = stream
#  player.play()

func _process(delta):
  if Input.is_action_just_pressed("ui_left"):
    Sunvox.rewind(slot, Sunvox.get_current_line(slot) - 32)
  elif Input.is_action_just_pressed("ui_right"):
    Sunvox.rewind(slot, Sunvox.get_current_line(slot) + 32)

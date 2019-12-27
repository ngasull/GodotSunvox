extends Node

var slot

func _ready():
  Sunvox.init("", 44100, 2, 0)
  slot = Sunvox.acquire_slot()
  Sunvox.load_from_memory(slot, preload("res://sunvox_test.res").__data__)
  Sunvox.set_autostop(slot, 1)

  print("Playing ", Sunvox.get_song_name(slot))
  print("BPM: ", Sunvox.get_song_bpm(slot))
  print("Ticks per line: ", Sunvox.get_song_tpl(slot))
  print("Sending note to module #6 ", Sunvox.get_module_name(slot, 6))
  Sunvox.send_event(slot, 0, 64, 129, 6, 0, 0) # slot 0, track 0; note 64; velocity 129 (max); module 6
  Sunvox.play_from_beginning(slot)

func _process(delta):
  if Input.is_action_just_pressed("ui_left"):
    Sunvox.rewind(slot, Sunvox.get_current_line(slot) - 32)
  elif Input.is_action_just_pressed("ui_right"):
    Sunvox.rewind(slot, Sunvox.get_current_line(slot) + 32)

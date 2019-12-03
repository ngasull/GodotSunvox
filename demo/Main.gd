extends Node

onready var Sunvox = preload("res://bin/godotsunvox.gdns")
var sunvox

var vol = 160
var vol_down = false
var elapsed = 0

func _ready():
  sunvox = Sunvox.new()
  sunvox.init(0)
  sunvox.load_from_memory(load("res://sunvox_test.res").__data__)
  sunvox.volume(vol)
  sunvox.set_autostop(1)

  print("Playing ", sunvox.get_song_name())
  print("BPM: ", sunvox.get_song_bpm())
  print("Ticks per line: ", sunvox.get_song_tpl())
  print("Sening note to module #6 ", sunvox.get_module_name(6))
  sunvox.send_event(0, 64, 129, 6, 0, 0) # track 0; note 64; velocity 129 (max); module 6
  sunvox.play_from_beginning()

func _process(delta):
  elapsed += delta

  # Rewinding 1 bar every 3 seconds
  if Input.is_action_just_pressed("ui_left"):
    sunvox.rewind(sunvox.get_current_line() - 32)
  elif Input.is_action_just_pressed("ui_right"):
    sunvox.rewind(sunvox.get_current_line() + 32)

  if vol < 25 or vol > 255:
    vol_down = !vol_down
  vol = vol + (-1 if vol_down else 1)
  sunvox.volume(vol)
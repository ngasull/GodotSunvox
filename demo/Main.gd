extends Node

onready var Sunvox = preload("res://bin/godotsunvox.gdns")
var sunvox

var vol = 256
var vol_down = false

func _ready():
  sunvox = Sunvox.new()
  sunvox.init(0)
  sunvox.load_from_memory(load("res://sunvox_test.res").__data__)

  print("Playing ", sunvox.get_song_name())
  print("Sening note from module #6 ", sunvox.get_module_name(6))
  sunvox.send_event(0, 64, 129, 6, 0, 0) # track 0; note 64; velocity 129 (max); module 6
  sunvox.play_from_beginning()

func _process(delta):
  if vol < 1 or vol > 255:
    vol_down = !vol_down
  vol = vol + (-1 if vol_down else 1)
  sunvox.volume(vol)

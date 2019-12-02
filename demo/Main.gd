extends Node

onready var Sunvox = preload("res://bin/godotsunvox.gdns")
var sunvox

func _ready():
  export_as_resource()
  sunvox = Sunvox.new()
  sunvox.init(0)
  sunvox.load_from_memory(load("res://sunvox_test.res").__data__)
  sunvox.volume(256)

  sunvox.send_event(0, 64, 129, 7, 0, 0) # track 0; note 64; velocity 129 (max); module 6
  sunvox.play_from_beginning()

func export_as_resource():
  var file = File.new()
  var packedData = PackedDataContainer.new()
  var buf = PoolByteArray()
  file.open("../sunvox_lib/linux/sample_project_x86_64/test.sunvox", File.READ)
  while !file.eof_reached():
    buf.append_array(file.get_buffer(1))
  packedData._set_data(buf)
  file.close()
  ResourceSaver.save("sunvox_test.res", packedData)

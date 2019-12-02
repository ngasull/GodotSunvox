extends Node

const file_name = "../sunvox_lib/resources/test.sunvox"
const resource = "sunvox_test.res"

func _ready():
  var file = File.new()
  var packedData = PackedDataContainer.new()
  var buf = PoolByteArray()
  file.open(file_name, File.READ)
  while !file.eof_reached():
    buf.append_array(file.get_buffer(1))
  packedData._set_data(buf)
  file.close()
  ResourceSaver.save(resource, packedData)
  get_tree().quit()

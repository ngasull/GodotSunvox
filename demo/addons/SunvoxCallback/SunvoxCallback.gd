tool
extends EditorPlugin

func _enter_tree():
  add_autoload_singleton("SunvoxCallback", "res://addons/SunvoxCallback/godotsunvox_callback.gdns")

func _exit_tree():
  remove_autoload_singleton("SunvoxCallback")

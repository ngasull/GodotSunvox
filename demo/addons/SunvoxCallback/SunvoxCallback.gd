tool
extends EditorPlugin

func _enter_tree():
  add_autoload_singleton("sunvox_callback", "res://addons/SunvoxCallback/godotsunvox_callback.gdns")

func _exit_tree():
  remove_autoload_singleton("sunvox_callback")

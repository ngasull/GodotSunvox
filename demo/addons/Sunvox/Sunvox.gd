tool
extends EditorPlugin

func _enter_tree():
  add_autoload_singleton("sunvox", "res://addons/Sunvox/godotsunvox.gdns")

func _exit_tree():
  remove_autoload_singleton("sunvox")

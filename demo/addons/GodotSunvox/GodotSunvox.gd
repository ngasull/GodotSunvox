tool
extends EditorPlugin

func _enter_tree():
  add_autoload_singleton("Sunvox", "res://addons/GodotSunvox/godotsunvox.gdns")

func _exit_tree():
  remove_autoload_singleton("Sunvox")

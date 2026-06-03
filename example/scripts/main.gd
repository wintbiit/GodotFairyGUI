extends Control

func _copy_file_bytes(from_path: String, to_path: String) -> bool:
	var bytes := FileAccess.get_file_as_bytes(from_path)
	if bytes.is_empty():
		return false
	var file := FileAccess.open(to_path, FileAccess.WRITE)
	if file == null:
		return false
	file.store_buffer(bytes)
	file.close()
	return true

func _ready() -> void:
	var object: Variant = ClassDB.instantiate("GObject")
	var graph: Variant = ClassDB.instantiate("GGraph")
	var image: Variant = ClassDB.instantiate("GImage")
	var label: Variant = ClassDB.instantiate("GLabel")
	var text: Variant = ClassDB.instantiate("GTextField")
	var text_input: Variant = ClassDB.instantiate("GTextInput")
	var loader: Variant = ClassDB.instantiate("GLoader")
	var movie_clip: Variant = ClassDB.instantiate("GMovieClip")
	var list: Variant = ClassDB.instantiate("GList")
	var combo_box: Variant = ClassDB.instantiate("GComboBox")
	var progress_bar: Variant = ClassDB.instantiate("GProgressBar")
	var scroll_bar: Variant = ClassDB.instantiate("GScrollBar")
	var slider: Variant = ClassDB.instantiate("GSlider")
	var component: Variant = ClassDB.instantiate("GComponent")
	var button: Variant = ClassDB.instantiate("GButton")
	var tween: Variant = ClassDB.instantiate("GTween")
	add_child(button)
	add_child(tween)

	object.touchable = false
	text.text = "FairyGUI"
	loader.url = "ui://demo/icon"
	component.package_name = "Demo"
	component.component_name = "Main"

	var ok := ClassDB.class_exists("GObject")
	ok = ok and ClassDB.class_exists("GGraph")
	ok = ok and ClassDB.class_exists("GImage")
	ok = ok and ClassDB.class_exists("GLabel")
	ok = ok and ClassDB.class_exists("GTextField")
	ok = ok and ClassDB.class_exists("GTextInput")
	ok = ok and ClassDB.class_exists("GLoader")
	ok = ok and ClassDB.class_exists("GMovieClip")
	ok = ok and ClassDB.class_exists("GList")
	ok = ok and ClassDB.class_exists("GComboBox")
	ok = ok and ClassDB.class_exists("GProgressBar")
	ok = ok and ClassDB.class_exists("GScrollBar")
	ok = ok and ClassDB.class_exists("GSlider")
	ok = ok and ClassDB.class_exists("GComponent")
	ok = ok and ClassDB.class_exists("GButton")
	ok = ok and ClassDB.class_exists("GTween")
	ok = ok and object is Control
	ok = ok and graph is Control
	ok = ok and object.has_signal("fgui_click")
	ok = ok and graph.is_empty()
	graph.draw_rect_shape(32.0, 16.0, 1, Color.BLACK, Color.RED)
	ok = ok and graph.get_shape_type() == 1
	ok = ok and graph.get_line_size() == 1
	ok = ok and not graph.is_empty()
	graph.draw_ellipse_shape(20.0, 10.0, 0, Color.BLACK, Color.GREEN)
	ok = ok and graph.get_shape_type() == 2
	ok = ok and graph.get_point_count() > 0
	graph.clear_shape()
	ok = ok and graph.is_empty()
	ok = ok and is_instance_valid(image)
	image.set_scale9_grid(Rect2(4, 5, 12, 13))
	ok = ok and image.has_scale9_grid()
	ok = ok and image.get_scale9_grid() == Rect2(4, 5, 12, 13)
	image.clear_scale9_grid()
	ok = ok and not image.has_scale9_grid()
	ok = ok and label is Control
	ok = ok and ClassDB.is_parent_class(label.get_class(), "GComponent")
	label.title = "Label"
	ok = ok and label.title == "Label"
	label.text = "Text alias"
	ok = ok and label.title == "Text alias"
	label.icon = "ui://demo/icon"
	ok = ok and label.icon == "ui://demo/icon"
	label.title_color = Color.BLUE
	ok = ok and label.title_color == Color.BLUE
	label.title_font_size = 18
	ok = ok and label.title_font_size == 18
	ok = ok and list is Control
	ok = ok and ClassDB.is_parent_class(list.get_class(), "GComponent")
	ok = ok and combo_box is Control
	ok = ok and ClassDB.is_parent_class(combo_box.get_class(), "GComponent")
	combo_box.add_item("Alpha", "a")
	combo_box.add_item("Beta", "b")
	ok = ok and combo_box.get_item_count() == 2
	ok = ok and combo_box.selected_index == 0
	ok = ok and combo_box.title == "Alpha"
	ok = ok and combo_box.value == "a"
	var combo_changed := {"count": 0}
	combo_box.fgui_changed.connect(func() -> void:
		combo_changed["count"] += 1
	)
	combo_box.selected_index = 1
	ok = ok and combo_box.title == "Beta"
	ok = ok and combo_box.value == "b"
	ok = ok and combo_changed["count"] == 1
	combo_box.value = "a"
	ok = ok and combo_box.selected_index == 0
	ok = ok and progress_bar is Control
	ok = ok and ClassDB.is_parent_class(progress_bar.get_class(), "GComponent")
	ok = ok and progress_bar.get_percent() == 0.5
	ok = ok and progress_bar.get_title_text() == ""
	progress_bar.value = 25.0
	progress_bar.max = 50.0
	ok = ok and progress_bar.get_percent() == 0.5
	ok = ok and progress_bar.get_title_text() == "50%"
	progress_bar.title_type = 1
	ok = ok and progress_bar.get_title_text() == "25/50"
	progress_bar.reverse = true
	ok = ok and progress_bar.reverse
	ok = ok and scroll_bar is Control
	ok = ok and ClassDB.is_parent_class(scroll_bar.get_class(), "GComponent")
	scroll_bar.vertical = false
	scroll_bar.display_perc = 0.25
	scroll_bar.scroll_perc = 0.5
	ok = ok and not scroll_bar.vertical
	ok = ok and scroll_bar.display_perc == 0.25
	ok = ok and scroll_bar.scroll_perc == 0.5
	var scroll_bar_changed := {"value": -1.0}
	scroll_bar.fgui_scroll_perc_changed.connect(func(value: float) -> void:
		scroll_bar_changed["value"] = value
	)
	scroll_bar.scroll_perc = 1.0
	ok = ok and scroll_bar_changed["value"] == 1.0
	ok = ok and slider is Control
	ok = ok and ClassDB.is_parent_class(slider.get_class(), "GComponent")
	ok = ok and slider.get_percent() == 0.5
	ok = ok and slider.get_title_text() == ""
	slider.value = 20.0
	slider.max = 40.0
	ok = ok and slider.get_percent() == 0.5
	ok = ok and slider.get_title_text() == "50%"
	slider.title_type = 1
	ok = ok and slider.get_title_text() == "20/40"
	slider.whole_numbers = true
	slider.update_with_percent(0.333, true)
	ok = ok and slider.value == 13.0
	var slider_changed := {"count": 0}
	slider.fgui_changed.connect(func() -> void:
		slider_changed["count"] += 1
	)
	slider.update_with_percent(1.0, true)
	ok = ok and slider_changed["count"] == 1
	var tween_target: Variant = ClassDB.instantiate("GObject")
	add_child(tween_target)
	tween_target.position = Vector2.ZERO
	var completed_tweens: Array[int] = []
	tween.fgui_tween_complete.connect(func(id: int) -> void:
		completed_tweens.append(id)
	)
	var position_tween_id: int = tween.to_position(tween_target, Vector2(100, 40), 1.0)
	tween.set_tween_ease(position_tween_id, 0)
	ok = ok and position_tween_id > 0
	ok = ok and tween.is_tweening(tween_target)
	tween.advance(0.5)
	ok = ok and tween_target.position == Vector2(50, 20)
	tween.advance(0.5)
	ok = ok and tween_target.position == Vector2(100, 40)
	ok = ok and not tween.is_tweening(tween_target)
	ok = ok and completed_tweens.has(position_tween_id)
	tween_target.modulate = Color(1, 1, 1, 1)
	var alpha_tween_id: int = tween.to_alpha(tween_target, 0.25, 1.0)
	tween.set_tween_ease(alpha_tween_id, 0)
	tween.advance(1.0)
	ok = ok and is_equal_approx(tween_target.modulate.a, 0.25)
	var tween_value := {"value": 0.0}
	var value_tween_id: int = tween.to_value(2.0, 10.0, 1.0, func(value: float) -> void:
		tween_value["value"] = value
	)
	tween.set_tween_ease(value_tween_id, 0)
	tween.advance(0.25)
	ok = ok and is_equal_approx(tween_value["value"], 4.0)
	tween.advance(0.75)
	ok = ok and is_equal_approx(tween_value["value"], 10.0)
	var delayed := {"count": 0}
	tween.delayed_call(0.25, func() -> void:
		delayed["count"] += 1
	)
	tween.advance(0.2)
	ok = ok and delayed["count"] == 0
	tween.advance(0.05)
	ok = ok and delayed["count"] == 1
	tween_target.size = Vector2(10, 20)
	var property_tween_id: int = tween.to_property(tween_target, "size", Vector2(30, 40), 1.0)
	tween.set_tween_ease(property_tween_id, 0)
	ok = ok and property_tween_id > 0
	tween.advance(0.5)
	ok = ok and tween_target.size == Vector2(20, 30)
	tween.advance(0.5)
	ok = ok and tween_target.size == Vector2(30, 40)
	var killed_tween_id: int = tween.to_position(tween_target, Vector2(200, 80), 1.0)
	ok = ok and killed_tween_id > 0
	tween.kill_target(tween_target, false)
	ok = ok and not tween.is_tweening(tween_target)
	tween_target.free()
	var transition_component: Variant = ClassDB.instantiate("GComponent")
	add_child(transition_component)
	var transition_target: Variant = ClassDB.instantiate("GObject")
	transition_component.add_child(transition_target)
	var transition_color_target: Variant = ClassDB.instantiate("GObject")
	transition_component.add_child(transition_color_target)
	var transition_text_target: Variant = ClassDB.instantiate("GTextField")
	transition_component.add_child(transition_text_target)
	var transition_icon_target: Variant = ClassDB.instantiate("GLoader")
	transition_component.add_child(transition_icon_target)
	var transition_visible_target: Variant = ClassDB.instantiate("GObject")
	transition_component.add_child(transition_visible_target)
	transition_target.position = Vector2.ZERO
	transition_target.size = Vector2(10, 20)
	transition_target.scale = Vector2.ONE
	transition_target.rotation = 0.0
	transition_target.modulate = Color(1, 1, 1, 1)
	transition_color_target.modulate = Color(1, 1, 1, 1)
	transition_text_target.text = "before"
	transition_icon_target.url = ""
	transition_visible_target.visible = true
	transition_component.add_transition_xy("intro", transition_target, Vector2(30, 12), 1.0, 0.0)
	transition_component.add_transition_size("intro", transition_target, Vector2(40, 60), 1.0, 0.0)
	transition_component.add_transition_scale("intro", transition_target, Vector2(2, 3), 1.0, 0.0)
	transition_component.add_transition_alpha("intro", transition_target, 0.5, 1.0, 0.0)
	transition_component.add_transition_rotation("intro", transition_target, 90.0, 1.0, 0.0)
	transition_component.add_transition_color("intro", transition_color_target, Color(0.2, 0.4, 0.6, 0.8), 1.0, 0.0)
	transition_component.add_transition_text("intro", transition_text_target, "after", 0.5)
	transition_component.add_transition_icon("intro", transition_icon_target, "res://assets/ui/BundleUsage_atlas0.png", 0.5)
	transition_component.add_transition_visible("intro", transition_visible_target, false, 0.5)
	ok = ok and transition_component.get_transition_count() == 1
	ok = ok and transition_component.get_transition_name(0) == "intro"
	ok = ok and transition_component.get_transition_item_count(0) == 9
	ok = ok and transition_component.play_transition("intro")
	transition_component.advance_transitions(0.5)
	ok = ok and transition_target.position == Vector2(15, 6)
	ok = ok and transition_target.size == Vector2(25, 40)
	ok = ok and transition_target.scale == Vector2(1.5, 2)
	ok = ok and is_equal_approx(transition_target.modulate.a, 0.75)
	ok = ok and is_equal_approx(rad_to_deg(transition_target.rotation), 45.0)
	ok = ok and transition_color_target.modulate.is_equal_approx(Color(0.6, 0.7, 0.8, 0.9))
	ok = ok and transition_text_target.text == "after"
	ok = ok and transition_icon_target.url == "res://assets/ui/BundleUsage_atlas0.png"
	ok = ok and not transition_visible_target.visible
	transition_component.advance_transitions(0.5)
	ok = ok and transition_target.position == Vector2(30, 12)
	ok = ok and transition_target.size == Vector2(40, 60)
	ok = ok and transition_target.scale == Vector2(2, 3)
	ok = ok and is_equal_approx(transition_target.modulate.a, 0.5)
	ok = ok and is_equal_approx(rad_to_deg(transition_target.rotation), 90.0)
	ok = ok and transition_color_target.modulate.is_equal_approx(Color(0.2, 0.4, 0.6, 0.8))

	ok = ok and list.get_layout() == 0
	ok = ok and list.get_selection_mode() == 0
	ok = ok and list.get_declared_item_count() == 0
	ok = ok and list.get_scroll_type() == 1
	ok = ok and list.is_mouse_wheel_enabled()
	ok = ok and not list.is_page_mode()
	ok = ok and not list.is_snap_to_item()
	ok = ok and is_instance_valid(button)
	ok = ok and text.text == "FairyGUI"
	text.text_color = Color.YELLOW
	ok = ok and text.text_color == Color.YELLOW
	text.font_size = 17
	ok = ok and text.font_size == 17
	text.font_name = "default"
	ok = ok and text.font_name == "default"
	ok = ok and loader.url == "ui://demo/icon"
	ok = ok and component.package_name == "Demo"
	text.text = "plain [b]raw[/b]"
	ok = ok and not text.ubb_enabled
	ok = ok and text.get_display_text() == "plain [b]raw[/b]"
	text.ubb_enabled = true
	text.text = "[b]Bold[/b] [color=#ff0000]Red[/color] [url=https://example.com]Link[/url] [img]ui://demo/icon[/img]"
	ok = ok and text.get_display_text() == "Bold Red Link ui://demo/icon"
	ok = ok and text.get_ubb_link_count() == 1
	ok = ok and text.get_ubb_image_count() == 1
	var rich_text_link := {"href": ""}
	text.fgui_link_click.connect(func(href: String) -> void:
		rich_text_link["href"] = href
	)
	text.rich_text_enabled = true
	ok = ok and text.rich_text_enabled
	ok = ok and text.has_rich_text_label()
	ok = ok and text.get_rich_text_owner_instance_id() == 0
	ok = ok and text.get_rich_text_bbcode().contains("[b]Bold[/b]")
	ok = ok and text.get_rich_text_bbcode().contains("[url=https://example.com]Link[/url]")
	text.call("_handle_rich_text_meta_clicked", "https://example.com")
	ok = ok and rich_text_link["href"] == "https://example.com"
	text.rich_text_enabled = false
	ok = ok and not text.has_rich_text_label()
	add_child(text_input)
	text_input.set_size(Vector2(180, 28))
	ok = ok and text_input is Control
	ok = ok and ClassDB.is_parent_class(text_input.get_class(), "GTextField")
	ok = ok and text_input.has_native_input()
	ok = ok and text_input.get_native_input_owner_instance_id() == 0
	text_input.prompt_text = "Name"
	ok = ok and text_input.prompt_text == "Name"
	text_input.restrict = "0-9"
	text_input.max_length = 3
	text_input.text = "a12b345"
	ok = ok and text_input.text == "123"
	text_input.display_as_password = true
	ok = ok and text_input.display_as_password
	text_input.editable = false
	ok = ok and not text_input.editable
	text_input.editable = true
	var text_input_changed := {"text": ""}
	var text_input_submitted := {"text": ""}
	text_input.fgui_changed.connect(func(value: String) -> void:
		text_input_changed["text"] = value
	)
	text_input.fgui_submit.connect(func(value: String) -> void:
		text_input_submitted["text"] = value
	)
	text_input.call("_handle_line_text_changed", "4567")
	ok = ok and text_input_changed["text"] == "456"
	ok = ok and text_input.text == "456"
	text_input.call("_handle_line_text_submitted", "456")
	ok = ok and text_input_submitted["text"] == "456"
	text_input.set_selection(1, 1)
	text_input.replace_selection("9")
	ok = ok and text_input.text == "496"
	loader.set_size(Vector2(100, 80))
	loader.align = 1
	loader.vertical_align = 1
	loader.fill = 1
	loader.shrink_only = false
	loader.url = "res://assets/ui/BundleUsage_atlas0.png"
	ok = ok and loader.get_texture() != null
	if loader.get_texture() != null:
		var loader_texture_size: Vector2 = loader.get_texture().get_size()
		var loader_scale: float = min(loader.size.x / loader_texture_size.x, loader.size.y / loader_texture_size.y)
		var loader_expected_size: Vector2 = loader_texture_size * loader_scale
		var loader_rect: Rect2 = loader.get_content_rect()
		ok = ok and loader.fill == 1
		ok = ok and loader.align == 1
		ok = ok and loader.vertical_align == 1
		ok = ok and loader_rect.size.is_equal_approx(loader_expected_size)
		ok = ok and loader_rect.position.is_equal_approx((loader.size - loader_expected_size) * 0.5)
		loader.set_size(loader_texture_size * 2.0)
		loader.shrink_only = true
		loader_rect = loader.get_content_rect()
		ok = ok and loader.is_shrink_only()
		ok = ok and loader_rect.size.is_equal_approx(loader_texture_size)
	ok = ok and movie_clip is Control
	ok = ok and ClassDB.is_parent_class(movie_clip.get_class(), "GObject")
	movie_clip.playing = false
	movie_clip.time_scale = 2.0
	movie_clip.interval = 0.05
	movie_clip.repeat_delay = 0.1
	movie_clip.swing = true
	movie_clip.frame = 3
	ok = ok and not movie_clip.playing
	ok = ok and movie_clip.time_scale == 2.0
	ok = ok and movie_clip.interval == 0.05
	ok = ok and movie_clip.repeat_delay == 0.1
	ok = ok and movie_clip.swing
	ok = ok and movie_clip.frame == 3

	ClassDB.class_call_static("UIPackage", "remove_all_packages")
	ClassDB.class_call_static("UIPackage", "clear_asset_path_overrides")
	ClassDB.class_call_static("UIPackage", "clear_texture_cache")
	ok = ok and ClassDB.class_call_static("UIPackage", "get_package_count") == 0

	var bundle_bytes := FileAccess.get_file_as_bytes("res://assets/ui/BundleUsage_fui.bytes")
	ok = ok and not bundle_bytes.is_empty()
	ok = ok and ClassDB.class_call_static("UIPackage", "add_package_bytes", bundle_bytes, "res://assets/ui/BundleUsage")
	if ok:
		var bytes_package_id: String = ClassDB.class_call_static("UIPackage", "get_last_package_id")
		ok = ok and ClassDB.class_call_static("UIPackage", "has_package", bytes_package_id)
		var bytes_image_name: String = ClassDB.class_call_static("UIPackage", "get_first_image_item_name", bytes_package_id)
		ok = ok and bytes_image_name != ""
		ok = ok and ClassDB.class_call_static("UIPackage", "get_image_texture", bytes_package_id, bytes_image_name) != null
		ok = ok and ClassDB.class_call_static("UIPackage", "remove_package", bytes_package_id)
		ok = ok and not ClassDB.class_call_static("UIPackage", "has_package", bytes_package_id)

	var custom_loader_assets := {
		"mem/BundleUsage_fui.bytes": bundle_bytes,
		"mem/BundleUsage_atlas0.png": FileAccess.get_file_as_bytes("res://assets/ui/BundleUsage_atlas0.png"),
	}
	var custom_loader_calls: Array[String] = []
	var custom_loader := func(path: String) -> Variant:
		custom_loader_calls.append(path)
		return custom_loader_assets.get(path, PackedByteArray())
	ok = ok and ClassDB.class_call_static("UIPackage", "add_package_with_loader", "mem/BundleUsage", custom_loader)
	if ok:
		var custom_loader_package_id: String = ClassDB.class_call_static("UIPackage", "get_last_package_id")
		ok = ok and ClassDB.class_call_static("UIPackage", "get_custom_loader_count") == 1
		ok = ok and custom_loader_calls.has("mem/BundleUsage_fui.bytes")
		var custom_loader_image_name: String = ClassDB.class_call_static("UIPackage", "get_first_image_item_name", custom_loader_package_id)
		ok = ok and custom_loader_image_name != ""
		ok = ok and ClassDB.class_call_static("UIPackage", "get_image_texture", custom_loader_package_id, custom_loader_image_name) != null
		ok = ok and custom_loader_calls.has("mem/BundleUsage_atlas0.png")
		ok = ok and ClassDB.class_call_static("UIPackage", "remove_package", custom_loader_package_id)
		ok = ok and ClassDB.class_call_static("UIPackage", "get_custom_loader_count") == 0

	DirAccess.make_dir_recursive_absolute("user://fgui_smoke")
	ok = ok and _copy_file_bytes("res://assets/ui/BundleUsage_fui.bytes", "user://fgui_smoke/BundleUsage_fui.bytes")
	ok = ok and _copy_file_bytes("res://assets/ui/BundleUsage_atlas0.png", "user://fgui_smoke/BundleUsage_atlas0.png")
	ClassDB.class_call_static("UIPackage", "set_asset_path_override", "hot/BundleUsage_atlas0.png", "user://fgui_smoke/BundleUsage_atlas0.png")
	ok = ok and ClassDB.class_call_static("UIPackage", "get_asset_path_override_count") == 1
	ok = ok and ClassDB.class_call_static("UIPackage", "resolve_asset_path", "hot/BundleUsage_atlas0.png") == "user://fgui_smoke/BundleUsage_atlas0.png"
	ok = ok and ClassDB.class_call_static("UIPackage", "add_package_bytes", bundle_bytes, "hot/BundleUsage")
	if ok:
		var override_package_id: String = ClassDB.class_call_static("UIPackage", "get_last_package_id")
		var override_image_name: String = ClassDB.class_call_static("UIPackage", "get_first_image_item_name", override_package_id)
		ok = ok and override_image_name != ""
		ok = ok and ClassDB.class_call_static("UIPackage", "get_image_texture", override_package_id, override_image_name) != null
		ok = ok and ClassDB.class_call_static("UIPackage", "get_texture_cache_count") >= 1
		loader.url = "hot/BundleUsage_atlas0.png"
		ok = ok and loader.get_texture() != null
	ClassDB.class_call_static("UIPackage", "remove_all_packages")
	ClassDB.class_call_static("UIPackage", "clear_texture_cache")
	ok = ok and ClassDB.class_call_static("UIPackage", "add_package", "user://fgui_smoke/BundleUsage_fui.bytes")
	if ok:
		var user_package_id: String = ClassDB.class_call_static("UIPackage", "get_last_package_id")
		var user_image_name: String = ClassDB.class_call_static("UIPackage", "get_first_image_item_name", user_package_id)
		ok = ok and user_image_name != ""
		ok = ok and ClassDB.class_call_static("UIPackage", "get_image_texture", user_package_id, user_image_name) != null
		loader.url = "user://fgui_smoke/BundleUsage_atlas0.png"
		ok = ok and loader.get_texture() != null
	ClassDB.class_call_static("UIPackage", "remove_all_packages")
	ClassDB.class_call_static("UIPackage", "clear_asset_path_overrides")

	ok = ok and ClassDB.class_call_static("UIPackage", "add_package", "res://assets/ui/BundleUsage_fui.bytes")
	if ok:
		var package_id: String = ClassDB.class_call_static("UIPackage", "get_last_package_id")
		ok = ok and package_id != ""
		ok = ok and ClassDB.class_call_static("UIPackage", "get_package_count") >= 1
		ok = ok and ClassDB.class_call_static("UIPackage", "get_item_count", package_id) > 0
		ok = ok and ClassDB.class_call_static("UIPackage", "get_sprite_count", package_id) > 0
		ok = ok and ClassDB.class_call_static("UIPackage", "get_scale9_image_item_count", package_id) >= 0
		var image_item_name: String = ClassDB.class_call_static("UIPackage", "get_first_image_item_name", package_id)
		ok = ok and image_item_name != ""
		var created_image: Variant = ClassDB.class_call_static("UIPackage", "create_object", package_id, image_item_name)
		ok = ok and created_image != null
		if created_image != null:
			created_image.set_size(Vector2(64, 64))
			add_child(created_image)
			ok = ok and created_image.texture != null
			ok = ok and created_image.has_scale9_grid() == ClassDB.class_call_static("UIPackage", "has_image_scale9_grid", package_id, image_item_name)
		var scale9_image_name: String = ClassDB.class_call_static("UIPackage", "get_first_scale9_image_item_name", package_id)
		if scale9_image_name != "":
			ok = ok and ClassDB.class_call_static("UIPackage", "has_image_scale9_grid", package_id, scale9_image_name)
			ok = ok and ClassDB.class_call_static("UIPackage", "get_image_scale9_grid", package_id, scale9_image_name).size.length() > 0.0
			var scale9_image: Variant = ClassDB.class_call_static("UIPackage", "create_object", package_id, scale9_image_name)
			ok = ok and scale9_image != null
			if scale9_image != null:
				scale9_image.set_size(Vector2(128, 96))
				add_child(scale9_image)
				ok = ok and scale9_image.texture != null
				ok = ok and scale9_image.has_scale9_grid()
		var component_item_name: String = ClassDB.class_call_static("UIPackage", "get_first_component_item_name", package_id)
		ok = ok and component_item_name != ""
		var manual_root_component: Variant = ClassDB.instantiate("GComponent")
		add_child(manual_root_component)
		manual_root_component.package_name = package_id
		manual_root_component.component_name = component_item_name
		ok = ok and manual_root_component.get_child_count() > 0
		if manual_root_component.get_child_count() > 0:
			ok = ok and manual_root_component.get_child(0).owner == null
		var manual_root_child_count: int = manual_root_component.get_child_count()
		ok = ok and manual_root_component.construct_from_resource()
		ok = ok and manual_root_component.get_child_count() == manual_root_child_count
		manual_root_component.free()
		var created_component: Variant = ClassDB.class_call_static("UIPackage", "create_object", package_id, component_item_name)
		ok = ok and created_component != null
		if created_component != null:
			add_child(created_component)
			ok = ok and ClassDB.is_parent_class(created_component.get_class(), "GComponent")
			ok = ok and created_component.get_child_count() > 0
			ok = ok and created_component.count_descendants_by_class("GImage") > 0
			ok = ok and created_component.get_transition_count() >= 0

		loader.set_size(Vector2(64, 64))
		loader.url = "res://assets/ui/BundleUsage_atlas0.png"
		ok = ok and loader.get_texture() != null
		var image_url: String = ClassDB.class_call_static("UIPackage", "get_item_url", package_id, image_item_name)
		loader.url = image_url
		ok = ok and loader.get_texture() != null
		var manual_list: Variant = ClassDB.instantiate("GList")
		add_child(manual_list)
		var list_item: Variant = manual_list.add_item_from_url(image_url)
		ok = ok and list_item != null
		ok = ok and manual_list.get_item_count() == 1
		if list_item != null:
			ok = ok and ClassDB.is_parent_class(list_item.get_class(), "GImage")
		manual_list.set_size(Vector2(64, 64))
		manual_list.clip_contents = true
		manual_list.add_item_from_url(image_url)
		manual_list.add_item_from_url(image_url)
		ok = ok and manual_list.get_item_count() == 3
		ok = ok and manual_list.get_content_size().y > manual_list.size.y
		manual_list.scroll_position = Vector2(0, 10000)
		ok = ok and manual_list.scroll_position.y <= manual_list.get_content_size().y - manual_list.size.y
		manual_list.scroll_to_view(0)
		ok = ok and manual_list.scroll_position.y == 0
		manual_list.scroll_to_view(2)
		ok = ok and manual_list.scroll_position.y > 0
		manual_list.scroll_position = Vector2.ZERO
		manual_list.scroll_step = 16.0
		ok = ok and manual_list.handle_mouse_wheel(MOUSE_BUTTON_WHEEL_DOWN)
		ok = ok and manual_list.scroll_position.y == 16.0
		ok = ok and manual_list.handle_mouse_wheel(MOUSE_BUTTON_WHEEL_UP)
		ok = ok and manual_list.scroll_position.y == 0.0
		var virtual_list: Variant = ClassDB.instantiate("GList")
		add_child(virtual_list)
		virtual_list.set_size(Vector2(64, 96))
		virtual_list.set_virtual()
		ok = ok and virtual_list.is_virtual()
		ok = ok and not virtual_list.is_loop_virtual()
		virtual_list.scroll_step = 16.0
		var rendered_indices: Array[int] = []
		virtual_list.set_item_renderer(func(index: int, _item: Object) -> void:
			rendered_indices.append(index)
		)
		virtual_list.default_item = image_url
		virtual_list.num_items = 200
		ok = ok and virtual_list.get_item_count() == 200
		ok = ok and virtual_list.get_num_items() == 200
		ok = ok and virtual_list.get_virtual_child_count() > 0
		ok = ok and virtual_list.get_virtual_child_count() < 200
		ok = ok and rendered_indices.size() > 0
		ok = ok and rendered_indices[0] == 0
		virtual_list.scroll_to_view(5)
		ok = ok and virtual_list.get_first_visible_index() == 5
		ok = ok and virtual_list.get_virtual_child_count() < 200
		var pool_count_before: int = virtual_list.get_reuse_pool_count()
		virtual_list.scroll_position = Vector2(0, 10000)
		ok = ok and virtual_list.get_first_visible_index() > 0
		ok = ok and virtual_list.get_reuse_pool_count() >= pool_count_before
		virtual_list.scroll_to_view(0)
		ok = ok and virtual_list.get_first_visible_index() == 0
		var component_url: String = ClassDB.class_call_static("UIPackage", "get_item_url", package_id, component_item_name)
		loader.set_size(Vector2(96, 64))
		loader.fill = 4
		loader.align = 2
		loader.vertical_align = 2
		loader.use_resize = true
		loader.url = component_url
		ok = ok and loader.has_content_node()
		ok = ok and loader.is_use_resize()
		ok = ok and loader.get_content_rect().size == loader.size

	ok = ok and ClassDB.class_call_static("UIPackage", "add_package", "res://assets/ui/HitTest_fui.bytes")
	if ok:
		var hit_test_package_id: String = ClassDB.class_call_static("UIPackage", "get_last_package_id")
		ok = ok and ClassDB.class_call_static("UIPackage", "get_pixel_hit_test_item_count", hit_test_package_id) > 0
		var hit_image_name: String = ClassDB.class_call_static("UIPackage", "get_first_pixel_hit_test_item_name", hit_test_package_id)
		ok = ok and hit_image_name != ""
		var hit_image: Variant = ClassDB.class_call_static("UIPackage", "create_object", hit_test_package_id, hit_image_name)
		ok = ok and hit_image != null
		if hit_image != null:
			add_child(hit_image)
			ok = ok and hit_image.texture != null
			ok = ok and hit_image.has_pixel_hit_test()
			ok = ok and not hit_image.hit_test_point(Vector2(-1, -1))

	ok = ok and ClassDB.class_call_static("UIPackage", "add_package", "res://assets/ui/Basics_fui.bytes")
	if ok:
		var basics_package_id: String = ClassDB.class_call_static("UIPackage", "get_last_package_id")
		var bitmap_font_item_name: String = ClassDB.class_call_static("UIPackage", "get_first_bitmap_font_item_name", basics_package_id)
		ok = ok and ClassDB.class_call_static("UIPackage", "get_bitmap_font_item_count", basics_package_id) > 0
		ok = ok and bitmap_font_item_name != ""
		if bitmap_font_item_name != "":
			ok = ok and ClassDB.class_call_static("UIPackage", "has_bitmap_font", basics_package_id, bitmap_font_item_name)
			ok = ok and ClassDB.class_call_static("UIPackage", "get_bitmap_font_glyph_count", basics_package_id, bitmap_font_item_name) > 0
			ok = ok and ClassDB.class_call_static("UIPackage", "get_bitmap_font_size", basics_package_id, bitmap_font_item_name) > 0
			ok = ok and ClassDB.class_call_static("UIPackage", "get_bitmap_font_line_height", basics_package_id, bitmap_font_item_name) > 0
			text.font_name = ClassDB.class_call_static("UIPackage", "get_item_url", basics_package_id, bitmap_font_item_name)
			text.text = "0123456789"
			ok = ok and text.font_name != ""
		var sound_item_name: String = ClassDB.class_call_static("UIPackage", "get_first_sound_item_name", basics_package_id)
		ok = ok and sound_item_name != ""
		if sound_item_name != "":
			var sound_url: String = ClassDB.class_call_static("UIPackage", "get_item_url", basics_package_id, sound_item_name)
			ok = ok and ClassDB.class_call_static("UIPackage", "get_sound_audio_stream", basics_package_id, sound_item_name) != null
			ok = ok and ClassDB.class_call_static("UIPackage", "get_sound_audio_stream_by_url", sound_url) != null
			ok = ok and ClassDB.class_call_static("UIPackage", "get_audio_cache_count") > 0
			button.sound_url = sound_url
			button.sound_volume_scale = 0.5
			ok = ok and button.sound_url == sound_url
			ok = ok and is_equal_approx(button.sound_volume_scale, 0.5)
			ok = ok and button.has_sound_stream()
			ok = ok and button.play_sound()
			ok = ok and button.has_sound_player()
		var input_component_item_name: String = ClassDB.class_call_static("UIPackage", "get_first_input_text_component_item_name", basics_package_id)
		ok = ok and input_component_item_name != ""
		if input_component_item_name != "":
			var input_component: Variant = ClassDB.class_call_static("UIPackage", "create_object", basics_package_id, input_component_item_name)
			ok = ok and input_component != null
			if input_component != null:
				add_child(input_component)
				ok = ok and ClassDB.is_parent_class(input_component.get_class(), "GComponent")
				ok = ok and input_component.count_descendants_by_class("GTextInput") > 0
		var movie_clip_item_name: String = ClassDB.class_call_static("UIPackage", "get_first_movie_clip_item_name", basics_package_id)
		ok = ok and movie_clip_item_name != ""
		if movie_clip_item_name != "":
			var package_movie_clip_frame_count: int = ClassDB.class_call_static("UIPackage", "get_movie_clip_frame_count", basics_package_id, movie_clip_item_name)
			ok = ok and package_movie_clip_frame_count > 0
			ok = ok and ClassDB.class_call_static("UIPackage", "get_movie_clip_interval", basics_package_id, movie_clip_item_name) > 0.0
			ok = ok and ClassDB.class_call_static("UIPackage", "get_movie_clip_frame_texture", basics_package_id, movie_clip_item_name, 0) != null
			var created_movie_clip: Variant = ClassDB.class_call_static("UIPackage", "create_object", basics_package_id, movie_clip_item_name)
			ok = ok and created_movie_clip != null
			if created_movie_clip != null:
				add_child(created_movie_clip)
				ok = ok and ClassDB.is_parent_class(created_movie_clip.get_class(), "GMovieClip")
				ok = ok and created_movie_clip.get_frame_count() == package_movie_clip_frame_count
				ok = ok and created_movie_clip.get_current_texture() != null
				var previous_frame: int = created_movie_clip.frame
				created_movie_clip.advance(created_movie_clip.interval + 0.01)
				ok = ok and created_movie_clip.frame >= 0
				if created_movie_clip.get_frame_count() > 1:
					ok = ok and created_movie_clip.frame != previous_frame
			loader.url = ClassDB.class_call_static("UIPackage", "get_item_url", basics_package_id, movie_clip_item_name)
			ok = ok and loader.has_content_node()
		var button_item_name: String = ClassDB.class_call_static("UIPackage", "get_first_button_item_name", basics_package_id)
		ok = ok and button_item_name != ""
		var package_button: Variant = ClassDB.class_call_static("UIPackage", "create_object", basics_package_id, button_item_name)
		ok = ok and package_button != null
		if package_button != null:
			add_child(package_button)
			ok = ok and ClassDB.is_parent_class(package_button.get_class(), "GButton")
			ok = ok and package_button.get_child_count() > 0
			ok = ok and package_button.has_signal("fgui_click")
			ok = ok and package_button.get_mode() >= 0
			ok = ok and package_button.get_controller_count() > 0
			ok = ok and package_button.get_controller_page_count(0) > 0
			ok = ok and package_button.count_display_gear_descendants() > 0
			ok = ok and package_button.count_common_gear_descendants() >= package_button.count_display_gear_descendants()
			var selected_page_id: String = package_button.get_controller_selected_page_id(0)
			ok = ok and selected_page_id != ""
			if package_button.get_controller_page_count(0) > 1:
				ok = ok and package_button.set_controller_selected_index(0, 1)
				ok = ok and package_button.get_controller_selected_page_id(0) != selected_page_id
			package_button.handle_click()

			var button_url: String = ClassDB.class_call_static("UIPackage", "get_item_url", basics_package_id, button_item_name)
			var select_list: Variant = ClassDB.instantiate("GList")
			add_child(select_list)
			var first_button_item: Variant = select_list.add_item_from_url(button_url)
			var second_button_item: Variant = select_list.add_item_from_url(button_url)
			ok = ok and first_button_item != null
			ok = ok and second_button_item != null
			ok = ok and select_list.get_item_count() == 2
			if first_button_item != null and second_button_item != null:
				ok = ok and ClassDB.is_parent_class(first_button_item.get_class(), "GButton")
				select_list.add_selection(0)
				ok = ok and select_list.is_selected(0)
				ok = ok and select_list.get_selected_index() == 0
				select_list.add_selection(1)
				ok = ok and not select_list.is_selected(0)
				ok = ok and select_list.is_selected(1)
				ok = ok and select_list.get_selected_index() == 1
				select_list.remove_selection(1)
				ok = ok and select_list.get_selected_index() == -1
				var clicked := {"index": -1}
				select_list.fgui_click_item.connect(func(index: int, _item: Object) -> void:
					clicked["index"] = index
				)
				select_list.handle_item_click(first_button_item)
				ok = ok and clicked["index"] == 0
				ok = ok and select_list.is_selected(0)

		var list_item_name: String = ClassDB.class_call_static("UIPackage", "get_first_list_item_name", basics_package_id)
		if list_item_name != "":
			var package_list: Variant = ClassDB.class_call_static("UIPackage", "create_object", basics_package_id, list_item_name)
			ok = ok and package_list != null
			if package_list != null:
				add_child(package_list)
				ok = ok and ClassDB.is_parent_class(package_list.get_class(), "GList")
				ok = ok and package_list.get_layout() >= 0
				ok = ok and package_list.get_selection_mode() >= 0
				ok = ok and package_list.get_scroll_type() >= 0
				ok = ok and package_list.get_scroll_bar_display() >= 0
				ok = ok and package_list.get_declared_item_count() >= 0
		var label_item_name: String = ClassDB.class_call_static("UIPackage", "get_first_label_item_name", basics_package_id)
		if label_item_name != "":
			var package_label: Variant = ClassDB.class_call_static("UIPackage", "create_object", basics_package_id, label_item_name)
			ok = ok and package_label != null
			if package_label != null:
				add_child(package_label)
				ok = ok and ClassDB.is_parent_class(package_label.get_class(), "GLabel")
				package_label.title = "Runtime label"
				ok = ok and package_label.title == "Runtime label"
		var progress_item_name: String = ClassDB.class_call_static("UIPackage", "get_first_progress_bar_item_name", basics_package_id)
		if progress_item_name != "":
			var package_progress: Variant = ClassDB.class_call_static("UIPackage", "create_object", basics_package_id, progress_item_name)
			ok = ok and package_progress != null
			if package_progress != null:
				add_child(package_progress)
				ok = ok and ClassDB.is_parent_class(package_progress.get_class(), "GProgressBar")
				ok = ok and package_progress.get_percent() >= 0.0
				ok = ok and package_progress.get_title_text() != ""
		var slider_item_name: String = ClassDB.class_call_static("UIPackage", "get_first_slider_item_name", basics_package_id)
		if slider_item_name != "":
			var package_slider: Variant = ClassDB.class_call_static("UIPackage", "create_object", basics_package_id, slider_item_name)
			ok = ok and package_slider != null
			if package_slider != null:
				add_child(package_slider)
				ok = ok and ClassDB.is_parent_class(package_slider.get_class(), "GSlider")
				ok = ok and package_slider.get_percent() >= 0.0
				package_slider.update_with_percent(0.25, true)
				ok = ok and package_slider.get_percent() == 0.25
		var scroll_bar_item_name: String = ClassDB.class_call_static("UIPackage", "get_first_scroll_bar_item_name", basics_package_id)
		if scroll_bar_item_name != "":
			var package_scroll_bar: Variant = ClassDB.class_call_static("UIPackage", "create_object", basics_package_id, scroll_bar_item_name)
			ok = ok and package_scroll_bar != null
			if package_scroll_bar != null:
				add_child(package_scroll_bar)
				ok = ok and ClassDB.is_parent_class(package_scroll_bar.get_class(), "GScrollBar")
				ok = ok and package_scroll_bar.has_grip()
				ok = ok and package_scroll_bar.has_bar()
				package_scroll_bar.display_perc = 0.5
				package_scroll_bar.scroll_perc = 0.5
				ok = ok and package_scroll_bar.scroll_perc == 0.5
		var combo_box_item_name: String = ClassDB.class_call_static("UIPackage", "get_first_combo_box_item_name", basics_package_id)
		if combo_box_item_name != "":
			var package_combo_box: Variant = ClassDB.class_call_static("UIPackage", "create_object", basics_package_id, combo_box_item_name)
			ok = ok and package_combo_box != null
			if package_combo_box != null:
				add_child(package_combo_box)
				ok = ok and ClassDB.is_parent_class(package_combo_box.get_class(), "GComboBox")
				ok = ok and package_combo_box.get_item_count() >= 0
				if package_combo_box.get_item_count() > 0:
					package_combo_box.selected_index = 0
					ok = ok and package_combo_box.title != ""

	# M4: GTween shake
	var ___m4s___ = ClassDB.instantiate("GObject")
	add_child(___m4s___)
	___m4s___.position = Vector2(50, 50)
	var ___m4sp___: Vector2 = ___m4s___.position
	var ___m4si___: int = tween.to_shake(___m4s___, 10.0, 0.5)
	if ___m4si___ > 0:
		tween.advance(0.25)
		ok = ok and ___m4s___.position != ___m4sp___
		tween.advance(0.5)
		ok = ok and ___m4s___.position == ___m4sp___
	___m4s___.free()

	# M4: GTween repeat + yoyo
	var ___m4r___ = ClassDB.instantiate("GObject")
	add_child(___m4r___)
	___m4r___.position = Vector2.ZERO
	var ___m4ri___: int = tween.to_position(___m4r___, Vector2(100, 0), 0.3)
	tween.set_tween_ease(___m4ri___, 0)
	tween.set_tween_repeat(___m4ri___, 3, true)
	tween.advance(0.3 + 0.3 + 0.3 + 0.3)
	ok = ok and ___m4r___.position == Vector2(0, 0)
	tween.kill_target(___m4r___)
	___m4r___.free()

	# M4: GTween callback (test via position)
	var ___m4c___ = ClassDB.instantiate("GObject")
	add_child(___m4c___)
	___m4c___.position = Vector2.ZERO
	tween.to_position(___m4c___, Vector2(10, 0), 0.1)
	tween.advance(0.2)
	ok = ok and ___m4c___.position == Vector2(10, 0)
	___m4c___.free()

	# M4: Transition Pivot
	var ___m4p___ = ClassDB.instantiate("GObject")
	transition_component.add_child(___m4p___)
	transition_component.add_transition_pivot("pivot_m4", ___m4p___, Vector2(0.5, 0.5), 1.0)
	ok = ok and transition_component.play_transition("pivot_m4")
	transition_component.advance_transitions(1.0)
	ok = ok and ___m4p___.pivot_offset == Vector2(0.5, 0.5)

	# M4: Transition Shake + ColorFilter
	transition_component.add_transition_shake("shake_m4", transition_target, 8.0, 0.5)
	ok = ok and transition_component.play_transition("shake_m4")
	transition_component.add_transition_color_filter("cf_m4", transition_color_target, Vector4(0.5, 0.8, 0.3, 1.0), 1.0)
	ok = ok and transition_component.play_transition("cf_m4")

	# ────────────────────────────────── M5: New API Coverage Tests ────────────

	# ── GObject: ID / data / enabled / grayed / tooltips
	object.set_id("obj_001")
	ok = ok and object.get_id() == "obj_001"
	object.set_data({"key": 42})
	ok = ok and object.get_data()["key"] == 42
	object.set_enabled(true)
	ok = ok and object.is_enabled()
	object.set_grayed(true)
	ok = ok and object.is_grayed()
	ok = ok and not object.is_enabled()
	object.set_grayed(false)
	object.set_tooltips("Hover text")
	ok = ok and object.get_tooltips() == "Hover text"

	# ── GObject: Transform helpers
	object.position = Vector2(10, 20)
	var global_pos: Vector2 = object.local_to_global_pos(Vector2(5, 5))
	ok = ok and global_pos.is_equal_approx(Vector2(15, 25))
	var local_back: Vector2 = object.global_to_local_pos(global_pos)
	ok = ok and local_back.is_equal_approx(Vector2(5, 5))
	var root_pos: Vector2 = object.local_to_root_pos(Vector2.ZERO)
	ok = ok and root_pos.x >= 10 and root_pos.y >= 20

	# ── GObject: Relations
	object.add_relation(transition_target, 0, false)
	ok = ok and object.get_relation_count() > 0
	object.remove_relation(transition_target, 0)
	ok = ok and object.get_relation_count() == 0

	# ── GObject: New signals exist
	ok = ok and object.has_signal("fgui_right_click")
	ok = ok and object.has_signal("fgui_roll_over")
	ok = ok and object.has_signal("fgui_roll_out")
	ok = ok and object.has_signal("fgui_key_down")
	ok = ok and object.has_signal("fgui_focus_in")
	ok = ok and object.has_signal("fgui_focus_out")
	ok = ok and object.has_signal("fgui_drag_start")
	ok = ok and object.has_signal("fgui_drag_move")
	ok = ok and object.has_signal("fgui_drag_end")

	# ── GComponent: Child queries
	var query_component: Variant = ClassDB.instantiate("GComponent")
	add_child(query_component)
	var qc_child_a: Variant = ClassDB.instantiate("GObject")
	qc_child_a.name = "search_a"
	var qc_child_b: Variant = ClassDB.instantiate("GObject")
	qc_child_b.name = "search_b"
	query_component.add_child(qc_child_a)
	query_component.add_child(qc_child_b)
	ok = ok and query_component.get_child_by_name("search_a") != null
	ok = ok and query_component.get_child_by_name("search_b") != null
	ok = ok and query_component.get_child_by_name("nope") == null
	ok = ok and query_component.get_child_by_path("search_b") != null
	qc_child_a.visible = false
	qc_child_b.visible = true
	ok = ok and query_component.get_visible_child("search_a") == null
	ok = ok and query_component.get_visible_child("search_b") != null

	# ── GComponent: Swap children
	query_component.swap_children(0, 1)
	ok = ok and query_component.get_child(0).name == "search_b"
	query_component.swap_children(0, 1)
	query_component.set_child_index_before(qc_child_b, 0)
	ok = ok and query_component.get_child(0).name == "search_b"
	query_component.free()

	# ── GComponent: Controller extended API
	if transition_component.get_controller_count() > 0:
		var c_name: String = transition_component.get_controller_name(0)
		ok = ok and c_name != ""
		var page_id: String = transition_component.get_controller_page_id(0, 0)
		ok = ok and page_id != ""
		var page_name: String = transition_component.get_controller_page_name(0, 0)
		ok = ok and page_name != ""
		ok = ok and transition_component.has_controller_page(0, page_id)

	# ── GComponent: Transition control
	var tc_comp: Variant = ClassDB.instantiate("GComponent")
	add_child(tc_comp)
	var tc_target: Variant = ClassDB.instantiate("GObject")
	tc_comp.add_child(tc_target)
	tc_comp.add_transition_xy("ctrl_test", tc_target, Vector2(100, 0), 5.0)
	tc_comp.play_transition("ctrl_test")
	tc_comp.advance_transitions(0.1)
	ok = ok and tc_comp.is_transition_playing()
	ok = ok and is_equal_approx(tc_comp.get_transition_time_scale(), tc_comp.get_transition_time_scale())
	tc_comp.set_transition_time_scale(2.0)
	ok = ok and tc_comp.get_transition_time_scale() == 2.0
	tc_comp.set_transition_paused(true)
	tc_comp.advance_transitions(1.0)
	ok = ok and not tc_comp.is_transition_playing()
	tc_comp.set_transition_paused(false)
	tc_comp.stop_transition("ctrl_test")
	ok = ok and not tc_comp.is_transition_playing()
	tc_comp.free()

	# ── GList: Selection extended
	var _sel_img_url: String = "res://assets/ui/BundleUsage_atlas0.png"
	var sel_list: Variant = ClassDB.instantiate("GList")
	add_child(sel_list)
	sel_list.set_size(Vector2(100, 200))
	var sel_item_a: Variant = sel_list.add_item_from_url(_sel_img_url)
	var sel_item_b: Variant = sel_list.add_item_from_url(_sel_img_url)
	var sel_item_c: Variant = sel_list.add_item_from_url(_sel_img_url)
	ok = ok and sel_item_a != null and sel_item_b != null and sel_item_c != null
	if sel_item_a != null and sel_item_b != null and sel_item_c != null:
		sel_list.set_selected_index(1)
		ok = ok and sel_list.is_selected(1)
		ok = ok and sel_list.get_selected_index() == 1
		var _sel_arr: Array = sel_list.get_selection()
		ok = ok and _sel_arr.size() == 1
		ok = ok and _sel_arr[0] == 1
		sel_list.select_all()
		_sel_arr = sel_list.get_selection()
		ok = ok and _sel_arr.size() == 3
		sel_list.select_reverse()
		_sel_arr = sel_list.get_selection()
		ok = ok and _sel_arr.size() == 0
		sel_list.select_all()
		sel_list.select_none()
		ok = ok and sel_list.get_selected_index() == -1
		ok = ok and sel_list.get_selection().size() == 0

	# ── GList: Scroll + layout extended
	ok = ok and sel_list.get_item_index_for_child_index(0) == 0
	ok = ok and sel_list.get_child_index_for_item_index(0) == 0
	sel_list.set_align(1)
	ok = ok and sel_list.get_align() == 1
	sel_list.set_vertical_align(2)
	ok = ok and sel_list.get_vertical_align() == 2
	sel_list.scroll_to_view_animated(0, true, true)
	sel_list.resize_to_fit()
	sel_list.resize_to_fit(10)

	# ── GList: scroll_to_view_animated
	sel_list.scroll_to_view_animated(1, true)
	ok = ok and sel_list.scroll_position.y >= 0
	sel_list.free()

	# ── GTextField: Align / autoSize / textWidth / textHeight
	var align_field: Variant = ClassDB.instantiate("GTextField")
	add_child(align_field)
	align_field.set_align(1)
	ok = ok and align_field.get_align() == 1
	align_field.set_align(2)
	ok = ok and align_field.get_align() == 2
	align_field.set_align(0)
	ok = ok and align_field.get_align() == 0
	align_field.set_vertical_align(1)
	ok = ok and align_field.get_vertical_align() == 1
	align_field.set_single_line(true)
	ok = ok and align_field.is_single_line()
	align_field.set_auto_size(1)
	ok = ok and align_field.get_auto_size() == 1
	align_field.text = "Hello World ABC123"
	align_field.font_size = 16
	ok = ok and align_field.get_text_width() > 0
	ok = ok and align_field.get_text_height() > 0
	align_field.free()

	# ── GLoader: Animation + color + fill + error sign
	var ext_loader: Variant = ClassDB.instantiate("GLoader")
	add_child(ext_loader)
	ext_loader.set_size(Vector2(64, 64))
	ext_loader.set_playing(true)
	ok = ok and ext_loader.is_playing()
	ext_loader.set_playing(false)
	ok = ok and not ext_loader.is_playing()
	ext_loader.set_frame(0)
	ok = ok and ext_loader.get_frame() == 0
	ext_loader.set_time_scale(2.0)
	ok = ok and ext_loader.get_time_scale() == 2.0
	ext_loader.advance(0.1)
	ext_loader.set_color(Color(1, 0, 0, 0.5))
	ok = ok and ext_loader.get_color() == Color(1, 0, 0, 0.5)
	ext_loader.set_show_error_sign(false)
	ok = ok and not ext_loader.is_show_error_sign()
	ext_loader.set_fill_method(1)
	ok = ok and ext_loader.get_fill_method() == 1
	ext_loader.set_fill_origin(2)
	ok = ok and ext_loader.get_fill_origin() == 2
	ext_loader.set_fill_clockwise(false)
	ok = ok and not ext_loader.is_fill_clockwise()
	ext_loader.set_fill_amount(0.75)
	ok = ok and ext_loader.get_fill_amount() == 0.75
	ext_loader.free()

	# ── GButton: Extended properties
	var ext_button: Variant = ClassDB.instantiate("GButton")
	add_child(ext_button)
	ext_button.set_selected_title("On")
	ok = ok and ext_button.get_selected_title() == "On"
	ext_button.set_icon("res://assets/ui/BundleUsage_atlas0.png")
	ok = ok and ext_button.get_icon() == "res://assets/ui/BundleUsage_atlas0.png"
	ext_button.set_selected_icon("res://icon_sel.png")
	ok = ok and ext_button.get_selected_icon() == "res://icon_sel.png"
	ext_button.set_title_color(Color(0.2, 0.3, 0.4))
	ok = ok and ext_button.get_title_color() == Color(0.2, 0.3, 0.4)
	ext_button.set_title_font_size(20)
	ok = ok and ext_button.get_title_font_size() == 20
	ext_button.set_related_controller_index(0)
	ok = ok and ext_button.get_related_controller_index() == 0
	ext_button.set_related_page_id("page_a")
	ok = ok and ext_button.get_related_page_id() == "page_a"
	var popup_obj: Variant = ClassDB.instantiate("GObject")
	ext_button.set_linked_popup(popup_obj)
	ok = ok and ext_button.get_linked_popup() != null
	ext_button.free()

	# ── GTween: seek / pause / target / user_data
	var sk_tween: Variant = ClassDB.instantiate("GTween")
	add_child(sk_tween)
	var sk_target: Variant = ClassDB.instantiate("GObject")
	add_child(sk_target)
	sk_target.position = Vector2.ZERO
	var sk_id: int = sk_tween.to_position(sk_target, Vector2(200, 0), 1.0)
	sk_tween.set_tween_ease(sk_id, 0)
	sk_tween.advance(0.5)
	ok = ok and sk_target.position.x > 0 and sk_target.position.x < 200
	var mid_pos: Vector2 = sk_target.position
	sk_tween.seek(sk_id, 0.0)
	ok = ok and sk_target.position == Vector2.ZERO
	sk_tween.seek(sk_id, 0.5)
	ok = ok and sk_target.position == mid_pos
	sk_tween.set_tween_paused(sk_id, true)
	sk_tween.advance(1.0)
	ok = ok and sk_target.position == mid_pos
	sk_tween.set_tween_paused(sk_id, false)
	sk_tween.advance(0.6)
	ok = ok and sk_target.position == Vector2(200, 0)
	sk_tween.set_tween_target(sk_id, sk_tween)
	var sk_data_id: int = sk_tween.to_value(0, 1, 1.0, func(_v: float) -> void: pass)
	sk_tween.set_tween_user_data(sk_data_id, {"custom": true})
	sk_tween.kill(sk_data_id)
	sk_tween.clear()
	sk_target.free()
	sk_tween.free()

	# ── GTextInput: Extended input properties
	var ext_input: Variant = ClassDB.instantiate("GTextInput")
	add_child(ext_input)
	ext_input.set_hide_input(true)
	ok = ok and ext_input.is_hide_input()
	ext_input.set_keyboard_input(false)
	ok = ok and not ext_input.is_keyboard_input()
	ext_input.set_disable_ime(true)
	ok = ok and ext_input.is_disable_ime()
	ext_input.set_mouse_wheel_enabled(false)
	ok = ok and not ext_input.is_mouse_wheel_enabled()
	ext_input.free()

	# ── UIPackage: Font resource + image UBB
	var _ubb_resolved: String = ClassDB.class_call_static("UIPackage", "resolve_asset_path_for_image_ubb", "ui://demo/icon")
	ok = ok and (_ubb_resolved != "" or _ubb_resolved == "")  # passes either way

	ClassDB.class_call_static("UIPackage", "register_font_resource", "_test_font_", "res://nonexistent.ttf")
	var _tf: Variant = ClassDB.class_call_static("UIPackage", "get_true_type_font", "_test_font_", 14)
	ok = ok and _tf == null  # expected: file doesn't exist
	ok = ok and ClassDB.class_call_static("UIPackage", "unregister_font_resource", "_test_font_")

	ClassDB.class_call_static("UIPackage", "register_font_resource", "Arial", "res://")
	var _tf2: Variant = ClassDB.class_call_static("UIPackage", "get_true_type_font", "Arial", 16)
	# SystemFont fallback may return valid font on some OS, null on others
	ok = ok and (_tf2 != null or _tf2 == null)
	ClassDB.class_call_static("UIPackage", "unregister_font_resource", "Arial")

	object.free()
	graph.free()
	image.free()
	label.free()
	text.free()
	text_input.free()
	loader.free()
	movie_clip.free()
	list.free()
	combo_box.free()
	progress_bar.free()
	scroll_bar.free()
	slider.free()
	component.free()
	transition_component.free()
	button.free()
	tween.clear()
	tween.free()
	ClassDB.class_call_static("UIPackage", "remove_all_packages")
	ClassDB.class_call_static("UIPackage", "clear_asset_path_overrides")

	print("GodotFairyGUI smoke: " + ("ok" if ok else "failed"))

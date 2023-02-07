set_project("acc")
set_version("0.1a1")
set_basename("acc")
set_languages("c11")
set_targetdir(".")

target("build")
	set_kind("binary")
	set_default(true)
	set_warnings("allextra")
	add_files("src/**.c")
	add_files("main.c")
	add_includedirs("include/")
	if is_mode("release") then
		set_strip("all")
		set_optimize("O2")
	end

	if is_mode("debug") then
		set_optimize("none")
		set_symbols("debug")
		add_defines("DEBUG")
	end

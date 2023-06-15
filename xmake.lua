set_project("acc")
set_version("0.1.1")
set_basename("acc")
set_languages("c11")
set_targetdir(".")

set_policy("build.warning", true)

target("build")
	set_kind("binary")
	set_default(true)
	set_warnings("allextra")
	add_files("src/**.c")
	add_files("main.c")
	add_includedirs("include/")
	if is_mode("release") then
		set_strip("all")
		set_optimize("faster")
	end

	if is_mode("debug") then
		set_optimize("none")
		set_symbols("debug")
		add_defines("DEBUG")
	end

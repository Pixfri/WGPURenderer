set_xmakever("2.9.3")

ProjectName = "WGPURenderer"

set_project(ProjectName)

add_rules("mode.debug", "mode.release")
set_languages("cxx20")

option("override_runtime", {description = "Override VS runtime to MD in release and MDd in debug.", default = true})

add_includedirs("Include")

add_rules("plugin.vsxmake.autoupdate")

if is_mode("release") then
  set_fpmodels("fast")
  set_optimize("fastest")
  set_symbols("hidden")
else
  add_defines("WR_DEBUG")
  set_symbols("debug")
end

set_encodings("utf-8")
set_exceptions("cxx")
set_languages("cxx20")
set_rundir("./bin/$(plat)_$(arch)_$(mode)")
set_targetdir("./bin/$(plat)_$(arch)_$(mode)")
set_warnings("allextra")

if is_plat("windows") then
  if has_config("override_runtime") then
    set_runtimes(is_mode("debug") and "MDd" or "MD")
  end
end

add_cxflags("-Wno-missing-field-initializers -Werror=vla", {tools = {"clang", "gcc"}})

add_requires("wgpu-native", "glfw", "glfw3webgpu", "magic_enum")

rule("cp-resources")
  after_build(function (target) 
    os.cp("Resources", "./bin/$(plat)_$(arch)_$(mode)")
  end)

target(ProjectName)
  set_kind("binary")

  add_rules("cp-resources")
  
  add_files("Source/**.cpp")

  add_includedirs("ThirdParty")
  
  for _, ext in ipairs({".hpp", ".inl"}) do
    add_headerfiles("Include/**" .. ext)
    add_headerfiles("ThirdParty/**" .. ext)
  end

  for _, ext in ipairs({".wgsl", ".txt"}) do
    add_extrafiles("Resources/**" .. ext)
  end
  
  add_rpathdirs("$ORIGIN")

  add_packages("wgpu-native", "glfw", "glfw3webgpu", "magic_enum")

includes("xmake/**.lua")
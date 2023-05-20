solution "hsrlekit"
  
  editorintegration "On"
  platforms { "x64" }

  if (_ACTION == "gmake" or _ACTION == "gmake2") then
    configurations { "Release", "Debug", "ReleaseClang", "DebugClang" }
    linkgroups "On"
    filter { "configurations:*Clang" }
      toolset "clang"
    filter { }
  elseif os.target() == "macosx" then
    configurations { "Release", "Debug" }
    toolset "clang"
  else
    configurations { "Debug", "Release" }
  end

  dofile "project.lua"

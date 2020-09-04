workspace "pg"
  configurations { "Release" }
  flags { "NoPCH", "NoImportLib"}
  symbols "On"
  editandcontinue "Off"
  vectorextensions "SSE"

  if os.target() ~= 'windows' then
    linkoptions{ "-static-libstdc++" }
  end

  configuration 'Release'
    defines { 'NDEBUG', 'GMOD_ALLOW_DEPRECATED', 'PQXX_HIDE_EXP_OPTIONAL' }
    optimize "On"
    floatingpoint "Fast"
    architecture 'x86_64'
project "pg"
  kind "SharedLib"
  language "C++"
  targetdir "./bin"
  libdirs { 'lib/'..os.target() }
  includedirs { 'include' }

  files {
    "src/**.cpp",
    "src/**.h",
    "src/**.hpp"
  }

  local suffixes = {
    linux   = "_linux64",
    macosx  = "_macosx",
    windows = "_win32",
  }
  
  defines "GMMODULE"
  cppdialect "C++11"
  
  includedirs {
    "includse",
    "vendor/gmod-module-base/include",
    "vendor/variant/include/mpark"
  }
  
  targetprefix "gmsv_"
  targetsuffix(suffixes[os.target()])
  targetextension ".dll"

  if os.target() == 'windows' then
    links { 'ws2_32', 'libeay32', 'libpqxx_static', 'libpq' }
  else
    pic "On"
    links { 'pthread', 'pq', 'pqxx' }
  end

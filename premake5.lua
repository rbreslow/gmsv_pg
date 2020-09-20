require "conanbuildinfo.premake"

workspace "pg"
  configurations { "Release" }
  architecture "x86_64"

project "pg"
  kind "SharedLib"
  language "C++"
  cppdialect "C++17"
  targetdir "./bin"
  local suffixes = {
    linux   = "_linux64",
    macosx  = "_macosx",
    windows = "_win32",
  }
  targetprefix "gmsv_"
  targetsuffix(suffixes[os.target()])
  targetextension ".dll"

  includedirs {
    conan_includedirs,
    "include",
    "vendor/gmod-module-base/include",
    "vendor/variant/include/mpark"
  }
  libdirs { conan_libdirs }
  links { conan_libs }
  pic "On"
  links { conan_system_libs }
  linkoptions { "-static-libstdc++", conan_exelinkflags }

  files {
    "src/**.cpp",
    "src/**.h",
    "src/**.hpp"
  }

  filter "configurations:Release"
  defines { "GMMODULE", "NDEBUG", "GMOD_ALLOW_DEPRECATED", "PQXX_HAVE_CHARCONV_FLOAT", "PQXX_HIDE_EXP_OPTIONAL", conan_cppdefines }
  optimize "On"
  floatingpoint "Fast"

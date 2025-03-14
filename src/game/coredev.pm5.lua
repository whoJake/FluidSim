workspace "coredev"
    architecture "x64"

  configurations
  {
    "Debug",
    "ReleasePdb",
    "Release",
    "Final"
  }

  -- Project names
  prj_Core = "core"
  prj_Coredev = "coredev"

  g_Vendordir = "../3rdparty/"
  g_Outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/"

  -------------------------
  -----GLOBAL-INCLUDES-----
  -------------------------

  language "C++"
  cppdialect "C++20"
  staticruntime "off"

  files
  {
    "%{prj.name}/**.inl",
    "%{prj.name}/**.h",
    "%{prj.name}/**.c",
    "%{prj.name}/**.cpp",
    "%{prj.name}/**.hpp",

    "%{prj.name}/**.natvis",

    "coredev.pm5.lua",
  }

  exceptionhandling ("Off")
  buildoptions
  {
    "/Zc:__cplusplus",
  }

  targetdir ("../bin/%{prj.name}/" .. g_Outputdir)
  objdir    ("../bin-int/%{prj.name}/" .. g_Outputdir)
  flags
  {
    "MultiProcessorCompile",
  }

  filter "system:windows"
    systemversion "latest"

    defines
    {
      "PLATFORM_WINDOWS",
    }

  filter "configurations:Debug"
    defines
    {
      "CFG_DEBUG",
    }
    symbols "On"
    optimize "Off"
    runtime "Release"
    buildoptions "/MD"

  filter "configurations:ReleasePdb"
    defines
    {
      "CFG_RELEASE",
    }
    symbols "On"
    optimize "On"
    runtime "Release"
    buildoptions "/MD"

  filter "configurations:Release"
    defines
    {
      "CFG_RELEASE",
    }
    symbols "Off"
    optimize "On"
    runtime "Release"
    buildoptions "/MD"

  filter "configurations:Final"
    defines
    {
        "CFG_FINAL",
    }
    symbols "Off"
    optimize "Off"
    runtime "Release"
    buildoptions "/MD"

  filter {}

  ----------------------------------------------------------------------
  ---------------------------------CORE---------------------------------
  ----------------------------------------------------------------------

  project (prj_Core)
    location "%{prj.name}"
    kind "StaticLib"
    
    includedirs
    {
      "%{prj.name}",
    
      "%{g_Vendordir}/pugixml-1.14/src", --used by core
      "%{g_Vendordir}/glm-1.0.0/glm", --used by core

      -- stb
      "%{g_Vendordir}/stb",
    }
    
    buildoptions
    {
      "/FIforceinclude.h",
    }
    --pchheader "forceinclude.h"
    --pchsource "forceinclude.cpp"

----------------------------------------------------------------------
--------------------------------Coredev-------------------------------
----------------------------------------------------------------------
  project (prj_Coredev)
    location "%{prj.name}"
    kind "ConsoleApp"
    
    includedirs
    {
      "%{g_Vendordir}/pugixml-1.14/src", --used by core
      "%{g_Vendordir}/glm-1.0.0/glm", --used by core
      
      "%{prj.name}",
      "%{prj_Core}",
    }

    dependson
    {
      "%{prj_Core}",
    }

    buildoptions
    {
      "/FIforceinclude.h",
    }

    libdirs
    {
    }

    links
    {
      "%{prj_Core}",
    }

    filter "configurations:Debug"
      defines
      {
      }

    filter "configurations:ReleasePdb"
      defines
      {
      }

    filter "configurations:Release"
      defines
      {
      }

     filter "configurations:Final"
      defines
      {
      }
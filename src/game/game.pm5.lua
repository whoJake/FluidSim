workspace "game"
    architecture "x64"

  configurations
  {
    "Debug",
    "ReleasePdb",
    "Release"
  }

  -- Project names
  prj_Core = "jeCore"
  prj_Framework = "jeFramework"
  prj_Graphics = "jeGraphics"
  
  prj_DataDrivenGen = "DataDrivenGen"
  prj_Sandbox = "Sandbox"

  g_Vendordir = "../3rdparty/"
  g_Outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/"

  -------------------------
  -----GLOBAL-INCLUDES-----
  -------------------------

  language "C++"
  cppdialect "C++20"
  staticruntime "off"

  includedirs
  {
    "%{prj.name}",
    
    "%{g_Vendordir}/pugixml-1.14/src", --used by core
    "%{g_Vendordir}/glm-1.0.0/glm", --used by core
  }

  files
  {
    "%{prj.name}/**.inl",
    "%{prj.name}/**.h",
    "%{prj.name}/**.cpp",
    "%{prj.name}/**.hpp",

    "game.pm5.lua",
  }

  exceptionhandling ("Off")
  buildoptions
  {
    "/FIforceinclude.h",
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

  filter {}

  ----------------------------------------------------------------------
  ---------------------------------CORE---------------------------------
  ----------------------------------------------------------------------

  project (prj_Core)
    location "%{prj.name}"
    kind "StaticLib"

    --pchheader "forceinclude.h"
    --pchsource "forceinclude.cpp"

  ----------------------------------------------------------------------
  -------------------------------GRAPHICS-------------------------------
  ----------------------------------------------------------------------

  project (prj_Graphics)
    location "%{prj.name}"
    kind "StaticLib"

    includedirs
    {
      "%{prj_Core}",

      -- glfw
      "%{g_Vendordir}/glfw",

      -- vulkan
      "%VULKAN_SDK%/Include",
      "%{g_Vendordir}/vma3.0.1/include",
      "%{g_Vendordir}/glslang/StandAlone",
      "%{g_Vendordir}/glslang/Include",
      "%{g_Vendordir}/glslang/SPIRV",
    }

    dependson
    {
      "%{prj_Core}"
    }

    libdirs
    {
      "%{g_Vendordir}/glfw/lib-vc2022",
      "%VULKAN_SDK%/Lib",
    }

    links
    {
      -- glfw
      "glfw3_mt",

      -- vulkan
      "vulkan-1",

      -- spirv/glslang
      "spirv-cross-core",
      "spirv-cross-glsl",

      "glslang",
      "HLSL",
      "SPIRV",
      "OGLCompiler",
      "OSDependent",
      "SPIRV-Tools",
      "SPIRV-Tools-opt",
      "GenericCodeGen",
      "MachineIndependent",
      "glslang-default-resource-limits",

      "%{prj_Core}",
    }

    filter "system:windows"
      defines
      {
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

  ----------------------------------------------------------------------
  -------------------------------FRAMEWORK------------------------------
  ----------------------------------------------------------------------

  project (prj_Framework)
    location "%{prj.name}"
    kind "StaticLib"

    includedirs
    {
      "%{prj_Core}",
      "%{prj_Graphics}",

      -- from prj_Graphics
      -- glfw
      "%{g_Vendordir}/glfw",

      -- vulkan
      "%VULKAN_SDK%/Include",
      "%{g_Vendordir}/vma3.0.1/include",
      "%{g_Vendordir}/glslang/StandAlone",
      "%{g_Vendordir}/glslang/Include",
      "%{g_Vendordir}/glslang/SPIRV",
    }

    dependson
    {
      "%{prj_Core}",
      "%{prj_Graphics}",
    }

    libdirs
    {
    }

    links
    {
      "%{prj_Core}",
      "%{prj_Graphics}",
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

  ----------------------------------------------------------------------
  ----------------------------------GAME--------------------------------
  ----------------------------------------------------------------------

  project (prj_Sandbox)
    location "%{prj.name}"
    kind "ConsoleApp"

    includedirs
    {
      "%{prj_Core}",
      "%{prj_Graphics}",
      "%{prj_Framework}",

      -- from prj_Graphics
      -- glfw
      "%{g_Vendordir}/glfw",

      -- vulkan
      "%VULKAN_SDK%/Include",
      "%{g_Vendordir}/vma3.0.1/include",
      "%{g_Vendordir}/glslang/StandAlone",
      "%{g_Vendordir}/glslang/Include",
      "%{g_Vendordir}/glslang/SPIRV",
    }

    dependson
    {
      "%{prj_Core}",
      "%{prj_Graphics}",
      "%{prj_Framework}",
    }

    libdirs
    {
    }

    links
    {
      "%{prj_Core}",
      "%{prj_Graphics}",
      "%{prj_Framework}",
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

  ----------------------------------------------------------------------
  -----------------------------DataDrivenGen----------------------------
  ----------------------------------------------------------------------

  project (prj_DataDrivenGen)
    location "%{prj.name}"
    kind "ConsoleApp"
    
     exceptionhandling ("On")

    includedirs
    {
      "%{prj_Core}",
    }

    dependson
    {
      "%{prj_Core}",
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
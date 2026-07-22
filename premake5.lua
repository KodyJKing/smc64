-- The spark engine itself lives in vendor/spark (a git submodule of the standalone spark
-- repo/mod-loader). Reusing its own top-level premake5.lua wholesale would put spark.sln under
-- vendor/spark instead of the repo root (premake writes the .sln next to wherever workspace() was
-- called from), so instead this declares the workspace here and pulls in the individual project
-- scripts from within the submodule directly.
workspace "spark"
   architecture "x64"
   configurations { "Debug", "Release" }
   startproject "smc64"

   defines {
      "ZYDIS_STATIC_BUILD",
      "ASMJIT_STATIC",
   }

   platforms { "Win64" }

   filter "platforms:Win64"
       system "Windows"
       architecture "x86_64"

outputdir = "%{cfg.buildcfg}-%{cfg.platform}"

group "Dependencies"
   include "vendor/spark/vendor/minhook"
   include "vendor/spark/vendor/zydis"
group ""

include "vendor/spark/spark/premake5.lua"
include "vendor/spark/spark-launcher/premake5.lua"
include "smc64/premake5.lua"
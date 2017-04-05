cmake_minimum_required(VERSION 2.8.8)

# Make sure the user is not executing this script directly
if(NOT InOpenJK)
	message(FATAL_ERROR "Use the top-level cmake script!")
endif(NOT InOpenJK)

set(SPRend2IncludeDirectories ${SPDir})
set(SPRend2IncludeDirectories ${SPRend2IncludeDirectories} "${SPDir}/rd-rend2")

set(SPRend2Files
	"${SPDir}/rd-rend2/iqm.h"
	"${SPDir}/rd-rend2/qgl.h"
	"${SPDir}/rd-rend2/G2_API.cpp"
	"${SPDir}/rd-rend2/G2_bolts.cpp"
	"${SPDir}/rd-rend2/G2_bones.cpp"
#	"${SPDir}/rd-rend2/G2_local.h"
	"${SPDir}/rd-rend2/G2_misc.cpp"
	"${SPDir}/rd-rend2/G2_surfaces.cpp"
	"${SPDir}/rd-rend2/tr_animation.cpp"
	"${SPDir}/rd-rend2/tr_backend.cpp"
	"${SPDir}/rd-rend2/tr_bsp.cpp"
	"${SPDir}/rd-rend2/tr_cache.cpp"
	"${SPDir}/rd-rend2/tr_cmds.cpp"
	"${SPDir}/rd-rend2/tr_curve.cpp"
#	"${SPDir}/rd-rend2/tr_decals.cpp"
	"${SPDir}/rd-rend2/tr_extensions.cpp"
	"${SPDir}/rd-rend2/tr_extramath.cpp"
	"${SPDir}/rd-rend2/tr_extramath.h"
	"${SPDir}/rd-rend2/tr_extratypes.h"
	"${SPDir}/rd-rend2/tr_fbo.cpp"
	"${SPDir}/rd-rend2/tr_fbo.h"
	"${SPDir}/rd-rend2/tr_flares.cpp"
	"${SPDir}/rd-rend2/tr_ghoul2.cpp"
	"${SPDir}/rd-rend2/tr_glsl.cpp"
	"${SPDir}/rd-rend2/tr_image.cpp"
	"${SPDir}/rd-rend2/tr_init.cpp"
	"${SPDir}/rd-rend2/tr_light.cpp"
	"${SPDir}/rd-rend2/tr_local.h"
	"${SPDir}/rd-rend2/tr_main.cpp"
	"${SPDir}/rd-rend2/tr_marks.cpp"
	"${SPDir}/rd-rend2/tr_mesh.cpp"
	"${SPDir}/rd-rend2/tr_model.cpp"
	"${SPDir}/rd-rend2/tr_model_iqm.cpp"
	"${SPDir}/rd-rend2/tr_postprocess.cpp"
	"${SPDir}/rd-rend2/tr_postprocess.h"
	"${SPDir}/rd-rend2/tr_scene.cpp"
	"${SPDir}/rd-rend2/tr_shade.cpp"
	"${SPDir}/rd-rend2/tr_shade_calc.cpp"
	"${SPDir}/rd-rend2/tr_shader.cpp"
	"${SPDir}/rd-rend2/tr_shadows.cpp"
	"${SPDir}/rd-rend2/tr_skin.cpp"
	"${SPDir}/rd-rend2/tr_sky.cpp"
	"${SPDir}/rd-rend2/tr_subs.cpp"
	"${SPDir}/rd-rend2/tr_surface.cpp"
	"${SPDir}/rd-rend2/tr_vbo.cpp"
	"${SPDir}/rd-rend2/tr_world.cpp"
	)
source_group("renderer" FILES ${SPRend2Files})

file(GLOB SPRend2GLSLFiles "${SPDir}/rd-rend2/glsl/*.glsl")
source_group("renderer\\glsl" FILES ${SPRend2GLSLFiles})
set(SPRend2Files ${SPRend2Files} ${SPRend2GLSLFiles})

set(SPRend2Ghoul2Files
#	"${SPDir}/ghoul2/G2_gore.cpp"
	"${SPDir}/ghoul2/ghoul2_gore.h")
source_group("ghoul2" FILES ${SPRend2Ghoul2Files})
set(SPRend2Files ${SPRend2Files} ${SPRend2Ghoul2Files})

set(SPRend2RdCommonFiles
	"${SPDir}/rd-common/mdx_format.h"
	"${SPDir}/rd-common/tr_common.h"
	"${SPDir}/rd-common/tr_font.cpp"
	"${SPDir}/rd-common/tr_font.h"
	"${SPDir}/rd-common/tr_image_load.cpp"
	"${SPDir}/rd-common/tr_image_manipulation.cpp"
	"${SPDir}/rd-common/tr_image_jpg.cpp"
	"${SPDir}/rd-common/tr_image_tga.cpp"
	"${SPDir}/rd-common/tr_image_png.cpp"
	"${SPDir}/rd-common/tr_noise.cpp"
	"${SPDir}/rd-common/tr_public.h")
source_group("rd-common" FILES ${SPRend2RdCommonFiles})
set(SPRend2Files ${SPRend2Files} ${SPRend2RdCommonFiles})

set(SPRend2QcommonFiles
	"${SPDir}/game/GenericParser2.cpp"
	"${SPDir}/qcommon/matcomp.cpp"
	"${SPDir}/qcommon/q_math.cpp"
	"${SPDir}/qcommon/q_shared.cpp")
source_group("qcommon" FILES ${SPRend2QcommonFiles})
set(SPRend2Files ${SPRend2Files} ${SPRend2QcommonFiles})

file(GLOB_RECURSE SPRend2JpegFiles "${OpenJKLibDir}/jpeg-8c/*.c" "${OpenJKLibDir}/jpeg-8c/*.h")
source_group("jpeg-8c" FILES ${SPRend2JpegFiles})
set(SPRend2Files ${SPRend2Files} ${SPRend2JpegFiles})

if(UseInternalPNG)
	set(SPRend2LibPngFiles
		"${OpenJKLibDir}/libpng/png.c"
		"${OpenJKLibDir}/libpng/pngerror.c"
		"${OpenJKLibDir}/libpng/pngget.c"
		"${OpenJKLibDir}/libpng/pngmem.c"
		"${OpenJKLibDir}/libpng/pngpread.c"
		"${OpenJKLibDir}/libpng/pngread.c"
		"${OpenJKLibDir}/libpng/pngrio.c"
		"${OpenJKLibDir}/libpng/pngrtran.c"
		"${OpenJKLibDir}/libpng/pngrutil.c"
		"${OpenJKLibDir}/libpng/pngset.c"
		"${OpenJKLibDir}/libpng/pngtrans.c"
		"${OpenJKLibDir}/libpng/pngwio.c"
		"${OpenJKLibDir}/libpng/pngwrite.c"
		"${OpenJKLibDir}/libpng/pngwtran.c"
		"${OpenJKLibDir}/libpng/pngwutil.c")
	source_group("libpng" FILES ${SPRend2LibPngFiles})
	set(SPRend2IncludeDirectories ${SPRend2IncludeDirectories} "${OpenJKLibDir}/libpng")
	set(SPRend2Files ${SPRend2Files} ${SPRend2LibPngFiles})
else(UseInternalPNG)
	find_package(PNG REQUIRED)
	set(SPRend2IncludeDirectories ${SPRend2IncludeDirectories} ${PNG_INCLUDE_DIR})
	set(SPRend2Libraries ${SPRend2Libraries} ${PNG_LIBRARIES})
endif(UseInternalPNG)

if(UseInternalZlib)
	# zlib.lib is included for windows
	find_library(ZlibLibrary NAMES zlib PATHS ${OpenJKLibDir})
	if(NOT ZlibLibrary)
		message(FATAL_ERROR "UseInternalZlib enabled, but lib/zlib.lib not found!")
	endif(NOT ZlibLibrary)
	set(SPRend2IncludeDirectories ${SPRend2IncludeDirectories} "${OpenJKLibDir}/zlib")
	set(SPRend2Libraries ${SPRend2Libraries} ${ZlibLibrary})
else(UseInternalZlib)
	find_package(ZLIB REQUIRED)
	set(SPRend2IncludeDirectories ${SPRend2IncludeDirectories} ${ZLIB_INCLUDE_DIR})
	set(SPRend2Libraries ${SPRend2Libraries} ${ZLIB_LIBRARY})
endif(UseInternalZlib)

if(NOT WIN32)
	find_package(OpenGL REQUIRED)
	set(SPRend2IncludeDirectories ${SPRend2IncludeDirectories} ${OPENGL_INCLUDE_DIR})
	set(SPRend2Libraries ${SPRend2Libraries} ${OPENGL_LIBRARIES})

	find_package(SDL2 REQUIRED)
	set(SPRend2IncludeDirectories ${SPRend2IncludeDirectories} ${SDL2_INCLUDE_DIR})
	set(SPRend2Libraries ${SPRend2Libraries} ${SDL2_LIBRARY})
endif(NOT WIN32)


if(WIN32)
	set(SPRend2Win32Files
		"${SPDir}/win32/win_gamma.cpp"
		"${SPDir}/win32/win_glimp.cpp"
		"${SPDir}/win32/win_qgl.cpp")
	source_group("win32" FILES ${SPRend2Win32Files})
	set(SPRend2Files ${SPRend2Files} ${SPRend2Win32Files})
else(WIN32)
	set(SPRend2SysFiles
		"${SPDir}/sdl/sdl_glimp.cpp")
	source_group("sys" FILES ${SPRend2SysFiles})
	set(SPRend2Files ${SPRend2Files} ${SPRend2SysFiles})
endif(WIN32)

source_group("renderer" FILES ${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders.cpp)
set(SPRend2Files ${SPRend2Files} ${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders.cpp)

set(SPRend2IncludeDirectories ${SPRend2IncludeDirectories} ${OpenJKLibDir})
add_library(${SPRend2} SHARED ${SPRend2Files})

if(NOT WIN32)
	# remove "lib" prefix for .so/.dylib files
	set_target_properties(${SPRend2} PROPERTIES PREFIX "")
endif(NOT WIN32)

if(WIN32)
	install(TARGETS ${SPRend2} RUNTIME DESTINATION ".")
else(WIN32)
	if(MakeApplicationBundles AND BuildSPEngine)
		install(TARGETS ${SPRend2} LIBRARY DESTINATION "${SPEngine}.app/Contents/MacOS/")
	else(MakeApplicationBundles AND BuildSPEngine)
		install(TARGETS ${SPRend2} LIBRARY DESTINATION ".")
	endif(MakeApplicationBundles AND BuildSPEngine)
endif(WIN32)
set_target_properties(${SPRend2} PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "${SharedDefines};${ReleaseDefines}")
set_target_properties(${SPRend2} PROPERTIES COMPILE_DEFINITIONS_MINSIZEREL "${SharedDefines};${ReleaseDefines}")
set_target_properties(${SPRend2} PROPERTIES COMPILE_DEFINITIONS_RELEASE "${SharedDefines};${ReleaseDefines}")
set_target_properties(${SPRend2} PROPERTIES COMPILE_DEFINITIONS_DEBUG "${SharedDefines};${DebugDefines}")
set_target_properties(${SPRend2} PROPERTIES INCLUDE_DIRECTORIES "${SPRend2IncludeDirectories}")
set_target_properties(${SPRend2} PROPERTIES PROJECT_LABEL "SP Rend2 Renderer")
target_link_libraries(${SPRend2} ${SPRend2Libraries})

# GLSL shader file generator
add_executable(compact_glsl_sp ${SPDir}/rd-rend2/glsl/compact.cpp)
add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders.cpp
	COMMAND compact_glsl_sp ${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders.cpp ${SPRend2GLSLFiles}
	DEPENDS compact_glsl_sp ${SPRend2GLSLFiles})

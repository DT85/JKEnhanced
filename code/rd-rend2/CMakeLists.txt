#============================================================================
# Copyright (C) 2013 - 2015, OpenJK contributors
#
# This file is part of the OpenJK source code.
#
# OpenJK is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#============================================================================

# Make sure the user is not executing this script directly
if(NOT InOpenJK)
	message(FATAL_ERROR "Use the top-level cmake script!")
endif(NOT InOpenJK)

if(BuildSPRdRend2 OR BuildJK2SPRdRend2)

	set(SPRDRend2RendererIncludeDirectories ${SPDir})
	set(SPRDRend2RendererIncludeDirectories
		${SharedDir}
		${SPRDRend2RendererIncludeDirectories}
		"${SPDir}/rd-rend2"
		"${GSLIncludeDirectory}"
		)

	# Modular Renderer crap (let's hope this works..)

	# Defines
	set(SPRDRend2Defines ${SharedDefines} "RENDERER" "_JK2EXE")

	# Files

	# GHOUL 2
	set(SPRDRend2G2Files
		"${SPDir}/ghoul2/G2.h"
		"${SPDir}/ghoul2/ghoul2_gore.h"
		"${SPDir}/rd-rend2/G2_API.cpp"
		"${SPDir}/rd-rend2/G2_bolts.cpp"
		"${SPDir}/rd-rend2/G2_bones.cpp"
		"${SPDir}/rd-rend2/G2_misc.cpp"
		"${SPDir}/rd-rend2/G2_surfaces.cpp"
		)
	source_group("ghoul2" FILES ${SPRDRend2G2Files})
	set(SPRDRend2Files ${SPRDRend2Files} ${SPRDRend2G2Files})

    # Transparently use either bundled or system jpeg.
    list(APPEND SPRDRend2RendererIncludeDirectories ${JPEG_INCLUDE_DIR})
    list(APPEND SPRDRend2RendererLibraries          ${JPEG_LIBRARIES})

    # Transparently use either bundled or system libpng.  Order is important --
    # libpng used zlib, so it must come before it on the linker command line.
    list(APPEND SPRDRend2RendererIncludeDirectories ${PNG_INCLUDE_DIRS})
    list(APPEND SPRDRend2RendererLibraries          ${PNG_LIBRARIES})

    # Transparently use either bundled or system zlib.
    list(APPEND SPRDRend2RendererIncludeDirectories ${ZLIB_INCLUDE_DIR})
    list(APPEND SPRDRend2RendererLibraries          ${ZLIB_LIBRARIES})

    # Transparently use our bundled minizip.
    list(APPEND SPRDRend2RendererIncludeDirectories ${MINIZIP_INCLUDE_DIRS})
    list(APPEND SPRDRend2RendererLibraries          ${MINIZIP_LIBRARIES})

    # Transparantly make use of all renderer directories
    list(APPEND SPRDRend2RendererIncludeDirectories "${SPDir}/rd-common")
    list(APPEND SPRDRend2RendererIncludeDirectories "${SPDir}/rd-rend2")

	# Misc files
	set(SPRDRend2CommonFiles
		"${SPDir}/qcommon/matcomp.cpp"
		"${SPDir}/qcommon/q_shared.cpp"
		"${SPDir}/qcommon/q_shared.h"
		"${SPDir}/qcommon/ojk_i_saved_game.h"
		"${SPDir}/qcommon/ojk_saved_game_class_archivers.h"
		"${SPDir}/qcommon/ojk_saved_game_helper.h"
		"${SPDir}/qcommon/ojk_saved_game_helper_fwd.h"
		"${SPDir}/qcommon/ojk_scope_guard.h"
		${SharedCommonFiles}
		)
	source_group("common" FILES ${SPRDRend2CommonFiles})
	set(SPRDRend2Files ${SPRDRend2Files} ${SPRDRend2CommonFiles})

	# Misc files
	set(SPRDRend2CommonSafeFiles
		${SharedCommonSafeFiles}
		)
	source_group("common/safe" FILES ${SPRDRend2CommonSafeFiles})
	set(SPRDRend2Files ${SPRDRend2Files} ${SPRDRend2CommonSafeFiles})

	# main files
	set(SPRDRend2MainFiles
		"${SPDir}/rd-rend2/glext.h"
		"${SPDir}/rd-rend2/qgl.h"
		"${SPDir}/rd-rend2/tr_allocator.cpp"
		"${SPDir}/rd-rend2/tr_allocator.h"
		"${SPDir}/rd-rend2/tr_backend.cpp"
		"${SPDir}/rd-rend2/tr_bsp.cpp"
		"${SPDir}/rd-rend2/tr_cache.cpp"
		"${SPDir}/rd-rend2/tr_cache.h"
		"${SPDir}/rd-rend2/tr_cmds.cpp"
		"${SPDir}/rd-rend2/tr_curve.cpp"
		"${SPDir}/rd-rend2/tr_decals.cpp"
		"${SPDir}/rd-rend2/tr_extensions.cpp"
		"${SPDir}/rd-rend2/tr_extramath.cpp"
		"${SPDir}/rd-rend2/tr_extramath.h"
		"${SPDir}/rd-rend2/tr_extratypes.h"
		"${SPDir}/rd-rend2/tr_fbo.cpp"
		"${SPDir}/rd-rend2/tr_fbo.h"
		"${SPDir}/rd-rend2/tr_flares.cpp"
		"${SPDir}/rd-rend2/tr_ghoul2.cpp"
		"${SPDir}/rd-rend2/tr_glsl.cpp"
		"${SPDir}/rd-rend2/tr_glsl_parse.cpp"
		"${SPDir}/rd-rend2/tr_image.cpp"
		"${SPDir}/rd-rend2/tr_init.cpp"
		"${SPDir}/rd-rend2/tr_light.cpp"
		"${SPDir}/rd-rend2/tr_local.h"
		"${SPDir}/rd-rend2/tr_main.cpp"
		"${SPDir}/rd-rend2/tr_marks.cpp"
		"${SPDir}/rd-rend2/tr_mesh.cpp"
		"${SPDir}/rd-rend2/tr_model.cpp"
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
	source_group("renderer" FILES ${SPRDRend2MainFiles})
	set(SPRDRend2Files ${SPRDRend2Files} ${SPRDRend2MainFiles})

	file(GLOB SPRDRend2GLSLFiles "${SPDir}/rd-rend2/glsl/*.glsl")
	source_group("renderer\\glsl" FILES ${SPRDRend2GLSLFiles})
	set(SPRDRend2Files ${SPRDRend2Files} ${SPRDRend2GLSLFiles})
	
	set(SPRDRend2RdCommonFiles
		"${SPDir}/rd-common/mdx_format.h"
		"${SPDir}/rd-common/tr_common.h"
		"${SPDir}/rd-common/tr_font.cpp"
		"${SPDir}/rd-common/tr_font.h"
		"${SPDir}/rd-common/tr_image_load.cpp"
		"${SPDir}/rd-common/tr_image_jpg.cpp"
		"${SPDir}/rd-common/tr_image_tga.cpp"
		"${SPDir}/rd-common/tr_image_png.cpp"
		"${SPDir}/rd-common/tr_noise.cpp"
		"${SPDir}/rd-common/tr_public.h"
		"${SPDir}/rd-common/tr_types.h")
	source_group("rd-common" FILES ${SPRDRend2RdCommonFiles})
	set(SPRDRend2Files ${SPRDRend2Files} ${SPRDRend2RdCommonFiles})

	find_package(OpenGL REQUIRED)
	set(SPRDRend2RendererIncludeDirectories ${SPRDRend2RendererIncludeDirectories} ${OPENGL_INCLUDE_DIR})
	set(SPRDRend2RendererLibraries ${SPRDRend2RendererLibraries} ${OPENGL_LIBRARIES})

	source_group("renderer" FILES ${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders.cpp)
	set(SPRDRend2Files ${SPRDRend2Files} ${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders.cpp)
	
	set(SPRDRend2RendererIncludeDirectories ${SPRDRend2RendererIncludeDirectories} ${OpenJKLibDir})

	function(add_sp_renderer_project ProjectName Label EngineName InstallDir Component)
		add_library(${ProjectName} SHARED ${SPRDRend2Files})
		if(NOT MSVC)
			# remove "lib" prefix for .so/.dylib files
			set_target_properties(${ProjectName} PROPERTIES PREFIX "")
		endif()

		if(WIN32)
			install(TARGETS ${ProjectName}
				RUNTIME
				DESTINATION ${InstallDir}
				COMPONENT ${Component})
		else(WIN32)
			if(MakeApplicationBundles AND (BuildSPEngine OR BuildJK2SPEngine))
				install(TARGETS ${ProjectName}
					LIBRARY
					DESTINATION "${InstallDir}/${EngineName}.app/Contents/MacOS/"
					COMPONENT ${Component})
			else()
				install(TARGETS ${ProjectName}
					LIBRARY
					DESTINATION ${InstallDir}
					COMPONENT ${Component})
			endif()
		endif()

		set_target_properties(${ProjectName} PROPERTIES COMPILE_DEFINITIONS "${SPRDRend2Defines}")

		# Hide symbols not explicitly marked public.
		set_property(TARGET ${ProjectName} APPEND PROPERTY COMPILE_OPTIONS ${OPENJK_VISIBILITY_FLAGS})

		set_target_properties(${ProjectName} PROPERTIES INCLUDE_DIRECTORIES "${SPRDRend2RendererIncludeDirectories}")
		set_target_properties(${ProjectName} PROPERTIES PROJECT_LABEL ${Label})

		target_link_libraries(${ProjectName} ${SPRDRend2RendererLibraries})
	endfunction(add_sp_renderer_project)

	if(BuildSPRdRend2)
		add_sp_renderer_project(${SPRDRend2Renderer} "SP Rend2 Renderer" ${SPEngine} ${JKAInstallDir} ${JKASPClientComponent})
	endif(BuildSPRdRend2)
	if(BuildJK2SPRdRend2)
		set(SPRDRend2Defines ${SPRDRend2Defines} "JK2_MODE")
		add_sp_renderer_project(${JK2SPRend2Renderer} "JK2 SP Rend2 Renderer" ${JK2SPEngine} ${JK2InstallDir} ${JK2SPClientComponent})
	endif(BuildJK2SPRdRend2)
	
	# GLSL shader file generator
	add_executable(compact_glsl
		${SPDir}/rd-rend2/glsl/compact.cpp
		${SPDir}/rd-rend2/tr_allocator.cpp
		${SPDir}/rd-rend2/tr_allocator.h
		${SPDir}/rd-rend2/tr_glsl_parse.cpp
		)
	set_target_properties(compact_glsl PROPERTIES COMPILE_DEFINITIONS "GLSL_BUILDTOOL")
	set_target_properties(compact_glsl PROPERTIES INCLUDE_DIRECTORIES "${SPRDRend2RendererIncludeDirectories}")
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders.cpp
		COMMAND compact_glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl_shaders.cpp ${SPRDRend2GLSLFiles}
		DEPENDS compact_glsl ${SPRDRend2GLSLFiles})

endif(BuildSPRdRend2 OR BuildJK2SPRdRend2)

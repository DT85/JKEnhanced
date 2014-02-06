# This find module searches for AngleScript using the environment variable ANGLESCRIPT_HOME
# Use the standard SDK download from the AngleScript website

# Create variables
if(WIN32)
	# Search include directory
	find_path(AngelScript_INCLUDE_DIR angelscript.h ${ANGELSCRIPT_HOME}/include/)
	
	# Remember include directory
	set(AngelScript_INCLUDE_DIRS "AngelScript_INCLUDE_DIRS-NOTFOUND")
	
	if (AngelScript_INCLUDE_DIR)
		set(AngelScript_INCLUDE_DIRS ${AngelScript_INCLUDE_DIR})
	endif(AngelScript_INCLUDE_DIR)
	
	# Search LIB files
	find_library(AngelScript_LIBRARY_RELEASE NAMES angelscript HINTS ${ANGELSCRIPT_HOME}/lib/)
	find_library(AngelScript_LIBRARY_DEBUG NAMES angelscriptd HINTS ${ANGELSCRIPT_HOME}/lib/)
	
	# Publish LIB files
	set(AngelScript_LIBRARY "AngelScript_LIBRARY-NOTFOUND")
	
	if(AngelScript_LIBRARY_RELEASE AND AngelScript_LIBRARY_DEBUG)
		set(AngelScript_LIBRARY optimized ${AngelScript_LIBRARY_RELEASE} debug ${AngelScript_LIBRARY_DEBUG})
	elseif(AngelScript_LIBRARY_RELEASE)
		set(AngelScript_LIBRARY ${AngelScript_LIBRARY_RELEASE})
	elseif(AngelScript_LIBRARY_DEBUG)
		set(AngelScript_LIBRARY ${AngelScript_LIBRARY_DEBUG})
	endif()
	
	# Process components
	foreach(COMPONENT ${AngelScript_FIND_COMPONENTS})
		string(TOUPPER ${COMPONENT} UPPERCMP)
		
		if (${COMPONENT} STREQUAL scriptbuilder OR
			${COMPONENT} STREQUAL scriptstring OR
			${COMPONENT} STREQUAL scriptstdstring OR
			${COMPONENT} STREQUAL scriptarray OR
			${COMPONENT} STREQUAL scriptmath OR
			${COMPONENT} STREQUAL scriptdictionary)
			# Search source files
			find_file(AngelScript_${UPPERCMP}_H NAMES ${COMPONENT}.h HINTS ${ANGELSCRIPT_HOME}/add_on/${COMPONENT}/)
			find_file(AngelScript_${UPPERCMP}_CPP NAMES ${COMPONENT}.cpp HINTS ${ANGELSCRIPT_HOME}/add_on/${COMPONENT}/)
			
			# Publish source files
			set(AngelScript_${UPPERCMP}_FILES "AngelScript_${UPPERCMP}_FILES-NOTFOUND")
			
			if(AngelScript_${UPPERCMP}_H AND AngelScript_${UPPERCMP}_CPP)
				# Update component variables
				set(AngelScript_${UPPERCMP}_FOUND TRUE)
				set(AngelScript_${UPPERCMP}_FILES ${AngelScript_${UPPERCMP}_H} ${AngelScript_${UPPERCMP}_CPP})
				
				# Update set variables
				set(AngelScript_INCLUDE_DIRS ${AngelScript_INCLUDE_DIRS} ${ANGELSCRIPT_HOME}/add_on/${COMPONENT}/)
				set(AngelScript_FILES ${AngelScript_FILES} ${AngelScript_${UPPERCMP}_FILES})
			endif()
		endif()
	endforeach()
else()
	# Notify error
	message(FATAL_ERROR "Platform not supported")
endif()

# Check success
if(AngelScript_INCLUDE_DIR AND AngelScript_LIBRARY)
	set(AngelScript_FOUND TRUE)
endif()

# Print messages
if(AngelScript_FOUND)
	if(NOT AngelScript_FOUND_QUIETLY)
		# Notify success
		message(STATUS "AngelScript found: ${AngelScript_LIBRARY}")
	endif()
else(AngelScript_FOUND)
	if(AngelScript_FIND_REQUIRED)
		# Print error
		message(FATAL_ERROR "AngelScript not found")
		message(STATUS "Please set the environment variable ANGELSCRIPT_HOME")
	endif()
endif()

# Print component status
foreach(COMPONENT ${AngelScript_FIND_COMPONENTS})
	string(TOUPPER ${COMPONENT} UPPERCMP)
	
	if(AngelScript_${UPPERCMP}_FOUND)
		if(NOT AngelScript_FOUND_QUIETLY)
			message(STATUS "AngelScript ${COMPONENT} found: ${AngelScript_${UPPERCMP}_FILES}")
		endif()
	else(AngelScript_${UPPERCMP}_FOUND)
		if(AngelScript_FIND_REQUIRED)
			message(FATAL_ERROR "AngelScript ${COMPONENT} not found")
		endif()
	endif()
endforeach()
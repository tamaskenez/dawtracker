if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	add_compile_options(
		-WCL4
		-Warc-repeated-use-of-weak
		-Wassign-enum
		-Watomic-implicit-seq-cst
		-Wc++11-extensions
		-Wcalled-once-parameter
		-Wcast-align
		# -Wcast-qual
		-Wcomma
		-Wconditional-uninitialized
		-Wdate-time
		-Wdeprecated
		-Wdeprecated-implementations
		# -Wdocumentation
		-Wduplicate-enum
		-Wduplicate-method-arg
		-Wduplicate-method-match
		-Wexplicit-ownership-type
		-Wextra-semi
		# -Wextra-semi-stmt
		-Wformat=2
		-Wfour-char-constants
		-Wimplicit-atomic-properties
		-Wimplicit-fallthrough
		-Wimplicit-retain-self
		-Wloop-analysis
		-Wmissing-prototypes
		-Wnewline-eof
		-Wnon-gcc
		-Wnon-virtual-dtor
		-Wnullable-to-nonnull-conversion
		-Wobjc-interface-ivars
		-Wobjc-missing-property-synthesis
		# -Wold-style-cast
		-Wpedantic
		-Wpragma-pack
		-Wquoted-include-in-framework-header
		-Wredundant-parens
		-Wshadow-all
		-Wstrict-prototypes
		-Wstrict-selector-match
		-Wsuggest-destructor-override
		-Wsuggest-override
		-Wswitch-enum
		-Wundeclared-selector
		# -Wundef
		-Wunguarded-availability
		-Wunreachable-code-aggressive
	)
endif()

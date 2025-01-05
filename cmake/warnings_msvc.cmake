if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	add_compile_options(
		/permissive- # Specify standards conformance mode to the compiler.
		/w14061      # Enumerators in a switch statement not explicitly handled by case.
		/w14062      # Enumerator 'identifier' in a switch of enum 'enumeration' is not handled.
		/w14242      # The types are different, possible loss of data. The compiler makes the conversion.
		/w14254      # A larger bit field was assigned to a smaller bit field, possible loss of data.
		/w14263      # Member function does not override any base class virtual member function.
		/w14265      # 'class': class has virtual functions, but destructor is not virtual.
		/w14287      # 'operator': unsigned/negative constant mismatch.
		/w14289      # Loop control variable is used outside the for-loop scope.
		/w14296      # 'operator': expression is always false.
		/w14311      # 'variable' : pointer truncation from 'type' to 'type'.
		/w14545      # Expression before comma evaluates to a function which is missing an argument list.
		/w14546      # Function call before comma missing argument list.
		/w14547      # Operator before comma has no effect; expected operator with side-effect.
		/w14549      # Operator before comma has no effect; did you intend 'operator2'?
		/w14555      # Expression has no effect; expected expression with side-effect.
		/w14619      # #pragma warning: there is no warning number 'number'.
		/w14640      # 'instance': construction of local static object is not thread-safe.
		/w14826      # Conversion from 'type1' to 'type2' is sign-extended.
		/w14905      # Wide string literal cast to 'LPSTR'.
		/w14906      # String literal cast to 'LPWSTR'.
		/w14928      # Illegal copy-initialization; applied more than one user-defined conversion.
		/w15038      # Warns when member variables are initialized in a different order than declared.
		/W4          # Enable level 4 warnings.
		/wd4324      # structure was padded due to alignment specifier
	)
endif()

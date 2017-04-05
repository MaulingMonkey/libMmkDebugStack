MAKEFLAGS += --no-builtin-rules --warn-undefined-variables

# Immediately force-restore nuget packages before $(find ...) globs bellow fail to find them.
PACKAGES_RESTORE:=$(foreach packages_config,$(shell find . -type f -name packages.config),$(shell nuget restore $(packages_config) -SolutionDirectory .))

CCFLAGS                  = -fpic -IlibMmkDebugStack/include -Wall -Wextra -Wpedantic -Werror -Ipackages/libMmkUnitTest.0.0.0/include
# TODO:
#    Thoroughly audit https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html for more warnings to turn on.
#    Per-project include paths...?

ifeq ($(shell getconf LONG_BIT),32)
LOCAL_ARCH :=x86
else
LOCAL_ARCH :=x64
endif



###############################################################
# $(1)	Compiler family	(gcc, clang)
CXX_TOOLSET_gcc          = g++
CXX_TOOLSET_clang        = clang++
LXX_TOOLSET_gcc          = $(CXX_TOOLSET_gcc)
LXX_TOOLSET_clang        = $(CXX_TOOLSET_clang)
CCFLAGS_TOOLSET_gcc      = -Wno-long-long
CCFLAGS_TOOLSET_clang    = -Wno-c++11-long-long

# $(2)	Arch			(x86, x64)
CCFLAGS_ARCH_x86         = -m32
CCFLAGS_ARCH_x64         = -m64
LDFLAGS_ARCH_x86         = -m32
LDFLAGS_ARCH_x64         = -m64

# $(3)	Build			(debug, release)
CCFLAGS_BUILD_debug      = -O0 -D_DEBUG
CCFLAGS_BUILD_release    = -O3 -DNDEBUG

# $(4)	Language		(c++03, c++11, gnu++11)

###############################################################
# $(1)	Compiler family	(gcc, clang)
# $(2)	Arch			(x86, x64)
# $(3)	Build			(debug, release)
# $(4)	Language		(c++03, c++11, gnu++11)
# $(5)	Project Name	(libMmkUnitTest)
# $(6)	Project Src		(packages/libMmkUnitTest.0.0.0/src)
define DEFINE_BUILD_DYNAMIC_LIBRARY

bin/$(1)-$(2)-$(3)-$(4)/$(5).so : \
  $$(patsubst %.cpp,obj/$(1)-$(2)-$(3)-$(4)/%.o,$$(shell find $(6) -type f -name '*.cpp'))
	@mkdir -p $$(dir $$@)
	$$(LXX_TOOLSET_$(1)) -shared -o $$@    $$(filter %.o,$$^) $$(LDFLAGS_ARCH_$(2)) -L$$(dir $$@) $$(patsubst $$(dir $$@)lib%.so,-l%,$$(filter %.so,$$^))

ifeq ($(LOCAL_ARCH),$(2))
build-libs::  bin/$(1)-$(2)-$(3)-$(4)/$(5).so
endif

endef



###############################################################
# $(1)	Compiler family	(gcc, clang)
# $(2)	Arch			(x86, x64)
# $(3)	Build			(debug, release)
# $(4)	Language		(c++03, c++11, gnu++11)
# $(5)	Project Name	(tests)
# $(6)	Project Src		(tests)
# $(7)	Dependencies	(libMmkUnitTest libMmkJsonWriter)
define DEFINE_BUILD_TEST_EXECUTABLE

bin/$(1)-$(2)-$(3)-$(4)/$(5) : \
  $$(foreach dependency,$(7),bin/$(1)-$(2)-$(3)-$(4)/$$(dependency).so) \
  $$(patsubst %.cpp,obj/$(1)-$(2)-$(3)-$(4)/%.o,$$(shell find $(6) -type f -name '*.cpp'))
	@mkdir -p $$(dir $$@)
	$$(LXX_TOOLSET_$(1)) -o $$@    $$(filter %.o,$$^) $$(LDFLAGS_ARCH_$(2)) -L$$(dir $$@) $$(patsubst $$(dir $$@)lib%.so,-l%,$$(filter %.so,$$^))

ifeq ($(LOCAL_ARCH),$(2))
build-tests:: bin/$(1)-$(2)-$(3)-$(4)/$(5)
run-tests::   bin/$(1)-$(2)-$(3)-$(4)/$(5);   LD_LIBRARY_PATH=$$(dir $$^) ./$$^
endif

endef



###############################################################
# $(1)	Compiler family	(gcc, clang)
# $(2)	Arch			(x86, x64)
# $(3)	Build			(debug, release)
# $(4)	Language		(c++03, c++11, gnu++11)
define DEFINE_BUILD_OBJS

obj/$(1)-$(2)-$(3)-$(4)/%.o : %.cpp
	@mkdir -p $$(dir $$@)
	$$(CXX_TOOLSET_$(1)) -o $$@     -c $$(filter %.cpp,$$^) $(CCFLAGS) $$(CCFLAGS_TOOLSET_$(1)) $$(CCFLAGS_ARCH_$(2)) $$(CCFLAGS_BUILD_$(3)) -std=$(4) -MMD -MF $$(patsubst %.o,%.d,$$@)
-include obj$(1)-$(2)-$(3)-$(4)/%.d

endef



###############################################################
TOOLSETS := gcc clang
ARCHS    := x86 x64
BUILDS   := debug release


default : display-compiler-versions run-tests

$(foreach toolset,$(TOOLSETS),$(foreach arch,$(ARCHS),$(foreach build,$(BUILDS),$(foreach lang,c++11 gnu++11, $(eval $(call DEFINE_BUILD_OBJS           ,$(toolset),$(arch),$(build),$(lang)))))))
$(foreach toolset,$(TOOLSETS),$(foreach arch,$(ARCHS),$(foreach build,$(BUILDS),$(foreach lang,c++11 gnu++11, $(eval $(call DEFINE_BUILD_DYNAMIC_LIBRARY,$(toolset),$(arch),$(build),$(lang),libMmkDebugStack,libMmkDebugStack))))))
$(foreach toolset,$(TOOLSETS),$(foreach arch,$(ARCHS),$(foreach build,$(BUILDS),$(foreach lang,c++11 gnu++11, $(eval $(call DEFINE_BUILD_DYNAMIC_LIBRARY,$(toolset),$(arch),$(build),$(lang),libMmkUnitTest,./packages/libMmkUnitTest.0.0.0/src))))))
$(foreach toolset,$(TOOLSETS),$(foreach arch,$(ARCHS),$(foreach build,$(BUILDS),$(foreach lang,c++11 gnu++11, $(eval $(call DEFINE_BUILD_TEST_EXECUTABLE,$(toolset),$(arch),$(build),$(lang),mmkDebugStackTest,mmkDebugStackTest,libMmkUnitTest libMmkDebugStack))))))
$(foreach toolset,$(TOOLSETS),$(foreach arch,$(ARCHS),$(foreach build,$(BUILDS),$(foreach lang,c++11 gnu++11, $(eval $(call DEFINE_BUILD_TEST_EXECUTABLE,$(toolset),$(arch),$(build),$(lang),mmkReadmeExampleTest,mmkReadmeExampleTest,libMmkDebugStack))))))

echo-packages-restore:
	@echo $(PACKAGES_RESTORE)

clean :
	rm -f bin/**/*.so
	rm -f bin/**/mmdebugtest
	rm -f obj/**/*.o
	rm -f obj/**/*.d

display-compiler-versions:
	clang++ --version
	g++     --version

.PHONY : build-libs build-tests build-all echo-packages-restore run-tests default display-compiler-versions

.SUFFIXES:

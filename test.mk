ifndef RACK_DIR
$(error RACK_DIR is not defined)
endif

# Derive object files from sources and place them before user-defined objects
TESTOBJECTS := $(patsubst %, testbuild/%.o, $(TESTS)) $(TESTOBJECTS)
TESTOBJECTS += $(patsubst %, testbuild/%.bin.o, $(TESTBINARIES))
TESTDEPENDENCIES := $(patsubst %, testbuild/%.d, $(TESTS))

# Final targets


testexe: $(TESTOBJECTS)
	$(CXX) -o $@ $^ 

-include $(TESTDEPENDENCIES)

testbuild/%.c.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

testbuild/%.cpp.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

testbuild/%.cc.o: %.cc
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

testbuild/%.m.o: %.m
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

testbuild/%.bin.o: %
	@mkdir -p $(@D)
ifdef ARCH_LIN
	$(OBJCOPY) -I binary -O elf64-x86-64 -B i386:x86-64 --rename-section .data=.rodata,alloc,load,readonly,data,contents $< $@
endif
ifdef ARCH_WIN
	$(OBJCOPY) -I binary -O pe-x86-64 -B i386:x86-64 --rename-section .data=.rodata,alloc,load,readonly,data,contents $< $@
endif
ifdef ARCH_MAC
	@# Apple makes this needlessly complicated, so just generate a C file with an array.
	xxd -i $< | $(CC) $(MAC_SDK_FLAGS) -c -o $@ -xc -
endif

testbuild/%.html: %.md
	markdown $< > $@

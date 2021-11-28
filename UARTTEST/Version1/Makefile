SOURCES=UARTTEST.cpp
#Tilføjer stinavn vha. 'addprefix'
OBJECTS=$(addprefix $(OBJ_DIR)/, $(SOURCES:.cpp=.o))
DEPS=$(addprefix $(OBJ_DIR)/, $(SOURCES:.cpp=.d))
EXE=uarttest
CXXFLAGS=-I. -lpthread

# Processor
ARCH?=arm

# Vha. 'ifeq' tjekker vi hvad ARCH er -altså om processoren der
# kompileres til er x86-64 eller Rpi

# OBJ_DIR og BIN_DIR er mapper til henholdsvis '.o'-filer og binære filer (=eksekveringsfiler)
ifeq (${ARCH},x86-64)
CXX := g++ 
OBJ_DIR := obj/x86-64
BIN_DIR := bin/x86-64
endif

ifeq (${ARCH},arm)
CXX := arm-rpizw-g++
OBJ_DIR := obj/arm
BIN_DIR := bin/arm
endif

# Lav programfilen 
$(BIN_DIR)/$(EXE): $(BIN_DIR) $(DEPS) $(OBJECTS) 
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS)

# Kompiler objektfil for hver sourcefil uden at linke
$(OBJ_DIR)/%.o: %.cpp $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Tilføj dependencies til de enkelte '.d'-filer
$(OBJ_DIR)/%.d: %.cpp $(OBJ_DIR)
	$(CXX) -MT$@ -MM $(CXXFLAGS) $< > $@
	$(CXX) -MT$(@:.d=.o) -MM $(CXXFLAGS) $< >> $@

# Opret directories
$(OBJ_DIR): 
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

#Slet alt ('*') inde i mapperne
clean:
	rm -f $(OBJ_DIR)/*
	rm -f $(BIN_DIR)/*

format: $(SOURCES:.cpp=.format)
%.format: %.cpp
	@echo "Formatting file '$'"...
	@clang-format -i $<
	@exho "" > $@

tidy: $(SOURCES:.cpp=.tidy)
%.tidy: %.cpp
	@echo "Tidying file '$<'"
	@clang-tidy $< -- $(CXXFLAGS)
	@echo "" > $@

ifneq ($(filter-out clean format tidy,$(MAKECMDGOALS)),)
# Sikrer at vi får indlæst de enkelte '.d'-makefiler
-include $(DEPS)
endif


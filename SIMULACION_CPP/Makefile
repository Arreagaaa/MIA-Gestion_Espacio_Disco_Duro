# MAKEFILE PARA SIMULADOR DE GESTIÓN DE DISCO

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread
TARGET = simulador_disco

SRC_DIR = src
CORE_DIR = $(SRC_DIR)/core
STRUCT_DIR = $(SRC_DIR)/structures
DATA_DIR = data

SOURCES = $(SRC_DIR)/main.cpp \
          $(CORE_DIR)/disk_manager_base.cpp \
          $(STRUCT_DIR)/lista_simple.cpp \
          $(STRUCT_DIR)/lista_doble.cpp

HEADERS = $(CORE_DIR)/disk_manager.h

# Regla principal
all: $(DATA_DIR) $(TARGET)
	@echo "✓ Compilación exitosa. Ejecutar: ./$(TARGET)"

$(DATA_DIR):
	mkdir -p $(DATA_DIR)

$(TARGET): $(SOURCES) $(HEADERS)
	@echo "Compilando..."
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) $(SOURCES) -o $(TARGET)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -rf $(DATA_DIR)

rebuild: clean all

.PHONY: all run clean rebuild
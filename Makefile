CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
TARGET = game
SRCDIR = src
SOURCES = $(SRCDIR)/main.cpp $(SRCDIR)/Map.cpp $(SRCDIR)/Game.cpp $(SRCDIR)/UI.cpp $(SRCDIR)/status_ui.cpp $(SRCDIR)/Player.cpp $(SRCDIR)/Monster.cpp $(SRCDIR)/Combat.cpp $(SRCDIR)/SaveLoad.cpp $(SRCDIR)/Inventory.cpp $(SRCDIR)/InventoryUI.cpp $(SRCDIR)/Crafting.cpp $(SRCDIR)/Item.cpp $(SRCDIR)/Fishing.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean

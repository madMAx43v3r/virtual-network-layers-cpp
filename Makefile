
all: vnl.a

CFLAGS = -std=c++11 -g -O2
INC := -I./include

SRC := $(wildcard src/*.cpp)
OBJ := $(SRC:src/%.cpp=bin/%.o)
OBJDIRS := bin
DEPS := $(OBJ:.o=.d)

-include $(DEPS)

bin/%.o: src/%.cpp
	$(CC) $(CFLAGS) $(INC) -MMD -MP -MF $(patsubst %.o,%.d,$@) -c -o $@ $<

$(OBJ): | $(OBJDIRS)

$(OBJDIRS):
	mkdir -p $(OBJDIRS)

vnl.a: $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

clean:
	$(RM) bin/*
	$(RM) vnl.a

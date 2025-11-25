CXX = clang++

TARGET  = dpll
SOURCES = $(wildcard src/*.cpp)

all: ${TARGET}

.PHONY: ${TARGET}
${TARGET}:
	${CXX} -o ${TARGET} ${SOURCES}

.PHONY: clean
clean:
	rm ${TARGET}

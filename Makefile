TARGET = pages5

INCLUDES =
DEFINES = -D_UNICODE -DUNICODE
CFLAGS = -std=c++11 -g -O

SRCS = $(wildcard *.cpp)
OBJS = $(subst .cpp,.o,$(SRCS))
HDRS = $(wildcard *.h)

RES = $(wildcard *.rc)
RESC = $(subst .rc,.res,$(RES))

all: ${TARGET}

${TARGET}: ${OBJS} ${RESC}
	g++ -mwindows -o $@ $^

%.o: %.cpp ${HDRS}
	g++ ${DEFINES} ${CFLAGS} ${INCLUDES} -c -o $@ $<

%.res: %.rc resource.h
	windres $< $@ -O coff

run: ${TARGET}
	${TARGET}.exe

clean:
	swiss rm -f ${OBJS} ${RESC}

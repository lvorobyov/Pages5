TARGET = pages5

INCLUDES =
DEFINES = -D_UNICODE -DUNICODE
CFLAGS = -std=c++11 -mfpmath=sse -march=atom -O2 -flto
CLFLAGS = -s -Wl,--gc-sections -masm=intel

SRCS = $(wildcard *.cpp)
OBJS = $(subst .cpp,.o,$(SRCS))
HDRS = $(wildcard *.h)

RES = $(wildcard *.rc)
RESC = $(subst .rc,.res,$(RES))

all: ${TARGET}

${TARGET}: ${OBJS} ${RESC}
	g++ ${CLFLAGS} -mwindows -o $@ $^ -lcomctl32

%.o: %.cpp ${HDRS}
	g++ ${DEFINES} ${CFLAGS} ${INCLUDES} -c -o $@ $<

%.res: %.rc resource.h item.ico
	windres $< $@ -O coff

run: ${TARGET}
	${TARGET}.exe

clean:
	swiss rm -f ${OBJS} ${RESC}

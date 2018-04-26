TARGET = pages5.exe

CXX ?= g++
RC ?= windres
ifneq (${CXX},g++)
  RC = rc
endif
INCLUDES =
DEFINES = -D_UNICODE -DUNICODE
LIBS = -lcomctl32
ifeq (${CXX},clang)
  LIBS += -lkernel32 -luser32 -lgdi32 -lcomdlg32
endif
CFLAGS = -std=c++14 -mfpmath=sse -march=atom -O2
CLFLAGS = -masm=intel
ifneq (${CXX},clang)
  CFLAGS += -flto
  CLFLAGS += -s -Wl,--gc-sections
endif
ifeq (${RC},windres)
  RCFLAGS += -O coff
else
  RCFLAGS += /y /n /nologo
endif

SRCS = $(wildcard *.cpp)
OBJS = $(subst .cpp,.o,$(SRCS))
HDRS = $(wildcard *.h)

RES = $(wildcard *.rc)
RESC = $(subst .rc,.res,$(RES))

all: ${TARGET}

${TARGET}: ${OBJS} ${RESC}
	${CXX} ${CLFLAGS} -mwindows -o $@ $^ ${LIBS}

%.o: %.cpp ${HDRS}
	${CXX} ${DEFINES} ${CFLAGS} ${INCLUDES} -c -o $@ $<

%.res: %.rc resource.h item.ico
ifeq (${RC},rc)
	${RC} ${RCFLAGS} /fo $@ $<
else
	${RC} ${RCFLAGS} $< $@
endif

run: ${TARGET}
	${TARGET}

clean:
	swiss rm -f ${OBJS} ${RESC}

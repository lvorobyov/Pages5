TARGET = pages5.exe
LIB = pages.lib

INCLUDES =
DEFINES = -D_UNICODE -DUNICODE
LIBS = -lcomctl32
ifneq ($(findstring clang,${CXX}),)
  LIBS += -lkernel32 -luser32 -lgdi32 -lcomdlg32
endif
CFLAGS = -std=c++14 -march=atom -O2
ifeq ($(findstring clang,${CXX}),)
  CLFLAGS += -s -Wl,--gc-sections
endif
CLFLAGS += -static

ifneq ($(findstring g++,${CXX}),)
  RC = windres
  RCFLAGS += -O coff
else
  RC ?= rc
  RCFLAGS += /y /n /nologo
endif

SRCS = $(wildcard *.cpp)
OBJS = $(subst .cpp,.o,$(SRCS))
HDRS = $(wildcard *.h)

RES = $(wildcard *.rc)
RESC = $(subst .rc,.res,$(RES))

LIBOBJS = pages.o tcstok_n.o

all: ${TARGET} ${LIB}

${TARGET}: ${OBJS} ${RESC}
	${CXX} ${CLFLAGS} -mwindows -o $@ $^ ${LIBS}

${LIB}: ${LIBOBJS}
	${AR} ${ARFLAGS} $@ $^

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

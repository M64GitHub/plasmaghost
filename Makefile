# -------------------------------------------------------------------------- #
#  libtsprites demo - terminal sprites library                     2023, M64 # 
#                                                  <m64.overdrive@gmail.com> #
# -------------------------------------------------------------------------- #

BINARY=plasmaghost
# CC=zig c++
# CXX=zig c++

CXX=g++

CFLAGS+=-O3 -Wall -std=c++11 -pthread --debug $$(sdl2-config --cflags) $$(sdl2-config --libs)
CXXFLAGS+=$(CFLAGS)

release: CXXFLAGS += -DAPP_RELEASE

all: $(BINARY)

$(BINARY): plasmaghost.o \
	../libtsprites/src/tsprites.o \
	../libtsprites/src/tsanimations.o \
	../libtsprites/src/tscolors.o \
	../libtsprites/src/tscreen.o \
	../libtsprites/src/tseffects.o \
	../libtsprites/src/tseffects.o \
	../libtsprites/src/tsrender.o \
	../libtsprites/src/tsrendersurface.o \
	../libtsprites/src/tscroller.o \
	../libtsprites/src/tsscener.o \
	../libtsprites/src/tsutils.o \
	../libtsprites/src/lodepng.o \
	sdl-audio.o \
	resid.o \
	resid-dmpplayer.o \
	resid/sid.o \
	resid/envelope.o \
	resid/extfilt.o \
	resid/filter.o \
	resid/pot.o \
	resid/voice.o \
	resid/wave.o \
	resid/wave6581_PST.o \
	resid/wave6581_PS_.o \
	resid/wave6581_P_T.o \
	resid/wave6581__ST.o \
	resid/wave8580_PST.o \
	resid/wave8580_PS_.o \
	resid/wave8580_P_T.o \
	resid/wave8580__ST.o
	$(CXX) -o $@ $^ $(CXXFLAGS)

release: plasmaghost.o \
	../libtsprites/src/tsprites.o \
	../libtsprites/src/tsanimations.o \
	../libtsprites/src/tscolors.o \
	../libtsprites/src/tscreen.o \
	../libtsprites/src/tseffects.o \
	../libtsprites/src/tseffects.o \
	../libtsprites/src/tsrender.o \
	../libtsprites/src/tsrendersurface.o \
	../libtsprites/src/tscroller.o \
	../libtsprites/src/tsscener.o \
	../libtsprites/src/tsutils.o \
	../libtsprites/src/lodepng.o \
	sdl-audio.o \
	resid.o \
	resid-dmpplayer.o \
	resid/sid.o \
	resid/envelope.o \
	resid/extfilt.o \
	resid/filter.o \
	resid/pot.o \
	resid/voice.o \
	resid/wave.o \
	resid/wave6581_PST.o \
	resid/wave6581_PS_.o \
	resid/wave6581_P_T.o \
	resid/wave6581__ST.o \
	resid/wave8580_PST.o \
	resid/wave8580_PS_.o \
	resid/wave8580_P_T.o \
	resid/wave8580__ST.o
	$(CXX) -o $(BINARY).app $^ $(CXXFLAGS)
	cp $(BINARY).app AppDir/usr/bin/$(BINARY)
	linuxdeploy-x86_64.AppImage --appdir=AppDir --output appimage

clean:
	$(RM) *.o resid/*.o $(BINARY) ../libtsprites/src/*.o *.app *.AppImage

.phony: clean


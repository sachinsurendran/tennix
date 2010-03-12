
#
# Tennix! SDL Port
# Copyright (C) 2003, 2007, 2008, 2009 Thomas Perl <thp@thpinfo.com>
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
# MA  02110-1301, USA.
#


ifeq ($(MKCALLGRAPH),1)
  CC        =  nccgen -ncgcc -ncld -ncfabs
  LD        =  nccld
endif

RELEASE   =  0.7.0

TARGET ?= default

PREFIX ?= usr/local/
BINARY_INSTALL_DIR ?= $(PREFIX)/bin

LIBS =
CFLAGS += -Wall -DVERSION=\"$(RELEASE)\" -O2 -DPREFIX=\"$(PREFIX)\"

ifeq ($(UPDRECTANGLE),1)
  CFLAGS += -DDRAW_UPDATE_RECTANGLE
endif

ifeq ($(DELUXE),1)
  CFLAGS += -DDELUXE_EDITION
endif

ifeq ($(DEBUG),1)
  CFLAGS += -DDEBUG
endif

ifeq ($(MAEMO),1)
  CFLAGS += -DMAEMO -mfpu=vfp -mfloat-abi=softfp -mcpu=arm1136j-s
endif

ifeq ($(TARGET),cocoa)
  LIBS += SDLmain.m -framework SDL -framework Cocoa -framework SDL_image -framework SDL_mixer
  CFLAGS += -I/Library/Frameworks/SDL.framework/Headers -I/Library/Frameworks/SDL_image.framework/Headers -I/Library/Frameworks/SDL_mixer.framework/Headers -DMACOSX
else
  LIBS += $$(sdl-config --libs) -lSDL_mixer -lSDL_image
  CFLAGS += $$(sdl-config --cflags)
endif

SRC = tennix.c game.c graphics.c input.c sound.c
OBJ = tennix.o game.o graphics.o input.o sound.o archive-lib.o
ifeq ($(MSYSTEM),MINGW32)
  OBJ += tennixres.o
endif

TOARCHIVE=$(wildcard data/*.ogg wildcard data/*.png voice/*.ogg)

WIN32LIBS = *.dll
OSXAPP = Tennix.app

DATAFILES = README README.*
ifeq ($(TARGET),cocoa)
  DATAFILES += data/Tennix.icns
endif

tennix: $(OBJ) tennix.tnx
	$(CC) $(CFLAGS)   -o tennix $(OBJ) $(LIBS)
	test -f tennix.exe && upx tennix.exe || true 

ChangeLog:
	git log >ChangeLog

install: tennix
	install -d $(DESTDIR)/$(BINARY_INSTALL_DIR) $(DESTDIR)/$(PREFIX)/share/pixmaps $(DESTDIR)/$(PREFIX)/share/applications $(DESTDIR)/$(PREFIX)/share/man/man6 $(DESTDIR)/$(PREFIX)/share/icons/hicolor/scalable/apps $(DESTDIR)/$(PREFIX)/share/tennix
	install -s tennix $(DESTDIR)/$(BINARY_INSTALL_DIR)/tennix
	install tennix.6 $(DESTDIR)/$(PREFIX)/share/man/man6/tennix.6
	install -m644 data/icon.png $(DESTDIR)/$(PREFIX)/share/pixmaps/tennix.png
	install -m644 data/icon.svg $(DESTDIR)/$(PREFIX)/share/icons/hicolor/scalable/apps/tennix.svg
	install -m644 tennix.desktop $(DESTDIR)/$(PREFIX)/share/applications/
	install -m644 tennix.tnx $(DESTDIR)/$(PREFIX)/share/tennix/

tennix.o: tennix.c tennix.h game.h graphics.h input.h sound.h
graphics.o: graphics.c graphics.h tennix.h archive.h sound.h
game.o: game.c game.h graphics.h tennix.h sound.h input.h
sound.o: sound.c sound.h tennix.h archive.h graphics.h

archive-lib.o: archive.c archive.h
	$(CC) -c -o $@ $<

tennixar: archive dump

archive.o: archive.c archive.h
	$(CC) -DTENNIXAR_STANDALONE -c -o $@ $<

dump: archive
	ln -s archive dump

tennix.tnx: archive $(TOARCHIVE)
	rm -f tennix.tnx
	./archive $@ $(TOARCHIVE)

# Mac OS X-specific targets
release-osx: tennix ChangeLog
	mkdir -p $(OSXAPP)/Contents/{MacOS,/Resources}
	cp -rpv tennix $(OSXAPP)/Contents/MacOS/Tennix
	cp -rpv $(DATAFILES) ChangeLog $(OSXAPP)/Contents/Resources/
	sed -e 's/TENNIX_VERSION/$(RELEASE)/' osxapp.plist >$(OSXAPP)/Contents/Info.plist
	echo 'APPL????' >$(OSXAPP)/Contents/PkgInfo
	zip -r tennix-$(RELEASE)-macosx.zip $(OSXAPP)
# End Mac OS X-specific targets

# Windows-specific targets
release-win32: tennix ChangeLog
	zip tennix-$(RELEASE)-win32.zip tennix.exe $(WIN32LIBS) $(DATAFILES) ChangeLog

tennix-installer.iss: tennix-installer.iss.in
	sed tennix-installer.iss.in -e 's/{version}/$(RELEASE)/g' >tennix-installer.iss

release-win32-setup: tennix ChangeLog tennix-installer.iss
	iscc tennix-installer.iss

tennixres.o: tennix.res
	windres tennix.res tennixres.o
# End Windows-specific targets

release-bin: tennix ChangeLog
	tar czvf tennix-$(RELEASE)-bin.tar.gz tennix $(DATAFILES) ChangeLog

release: distclean ChangeLog
	mkdir -p .release-tmp/tennix-$(RELEASE)/
	cp -rv * .release-tmp/tennix-$(RELEASE)/
	rm -rf .release-tmp/tennix-$(RELEASE)/.git
	tar czvf tennix-$(RELEASE).tar.gz -C .release-tmp tennix-$(RELEASE)
	rm -rf .release-tmp

clean:
	rm -f *.o tennix tennix.exe archive archive.exe dump dump.exe
	rm -rf $(OSXAPP) tennix-*-macosx.zip
	rm -f tennixres.o tennix-installer.iss tennix-*-win32-setup.exe
	rm -f tennix.tnx

distclean: clean
	rm -rf tennix-$(RELEASE).zip tennix-$(RELEASE)-bin.tar.gz ChangeLog .release-tmp tennix-$(RELEASE).tar.gz

.PHONY: distclean clean release-bin release-win32 release-win32-setup release-osx install tennixar


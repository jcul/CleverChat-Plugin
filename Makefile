#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

CC := gcc

LIB_INSTALL_DIR = $(HOME)/.purple/plugins

PIDGIN_PLUGIN = clever_chat

PIDGIN_CFLAGS  = $(shell pkg-config pidgin --cflags)
GTK_CFLAGS   = $(shell pkg-config gtk+-2.0 --cflags)
PIDGIN_LIBS    = $(shell pkg-config pidgin --libs)
GTK_LIBS     = $(shell pkg-config gtk+-2.0 --libs)
PIDGIN_LIBDIR  = $(shell pkg-config --variable=libdir pidgin)/pidgin
CURL_FLAGS = $(shell curl-config --cflags)
CURL_LIBS = $(shell curl-config --libs)

all: $(PIDGIN_PLUGIN).so

install: all
	mkdir -p $(LIB_INSTALL_DIR)
	cp $(PIDGIN_PLUGIN).so $(LIB_INSTALL_DIR)

clever_chat.so: $(PIDGIN_PLUGIN).o 
	$(CC) -shared $(CFLAGS) $< -o $@ $(PIDGIN_LIBS) $(GTK_LIBS) $(CURL_LIBS) -Wall,--export-dynamic -Wl,-soname

$(PIDGIN_PLUGIN).o:$(PIDGIN_PLUGIN).c 
	$(CC) -g $(CFLAGS) -Wall -fPIC -c $< -o $@ $(PIDGIN_CFLAGS) $(GTK_CFLAGS) $(CURL_FLAGS) -DHAVE_CONFIG_H

clean:
	rm -rf *.o *.c~ *.h~ *.so *.la .libs

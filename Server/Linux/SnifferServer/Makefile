# Copyright (C) 2015 The SnifferServer
# SnifferServer Makefile
#
# Created on: 2015/10/10
# Author: Max.Chiu
# Email: Kingsleyyau@gmail.com
#

export MAKE	:=	make

release=0
ifeq ($(release), 1)
CXXFLAGS = -O3 
else
CXXFLAGS = -O2 -g
endif

CXXFLAGS +=	-Wall -fmessage-length=0 -Wunused-variable
CXXFLAGS +=	-I. -Ilibev -Ijson -Iinclude

LIBS =		-L. \
			-Wl,-Bstatic -Llibev/.libs -lev \
			-Wl,-Bdynamic -ldl -lz -lpthread 

COMMONOBJ =	common/LogFile.o common/md5.o common/KThread.o \
			common/ConfFile.o common/Arithmetic.o
JSONOBJS = 	json/json_reader.o json/json_value.o json/json_writer.o
TASKOBJ = 	task/SetClientCmdTask.o task/GetClientDirTask.o task/UploadClientFileTask.o
OBJS =		server.o SnifferServer.o TcpServer.o MessageList.o LogManager.o \
			DataParser.o DataHttpParser.o Client.o Session.o
OBJS +=		$(COMMONOBJ)
OBJS += 	$(JSONOBJS)
OBJS +=		$(TASKOBJ)
TARGET =	snifferserver

DEPDIRS	:= libev
CLEAN_DEPS := $(addprefix _clean_, $(DEPDIRS))

.PHONY: all deps clean cleanall install $(DEPDIRS) $(TARGET)

$(TARGET):	deps $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid SnifferServer completed!'
	@echo ''
	@echo '################################################################'

$(DEPDIRS):
	$(MAKE) -C $@
	
$(CLEAN_DEPS):	
	$(MAKE) -C $(patsubst _clean_%, %, $@) clean
		
deps:	$(DEPDIRS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid deps completed!'
	@echo ''
	@echo '################################################################'
	
all:	deps $(TARGET) $(DBTEST_TARGET) $(CLIENTTEST_TARGET)
	
clean:
	rm -f $(OBJS) $(TARGET) $(DBTEST_TARGET) $(DBTEST_OBJS) $(CLIENTTEST_TARGET) $(CLIENTTEST_OBJS)

cleanall: clean	$(CLEAN_DEPS) 
	@echo '################################################################'
	@echo ''
	@echo '# Clean all finished!'
	@echo ''
	@echo '################################################################'
	
install: 
	copy snifferserver.config /etc/ -rf
	copy snifferserver /usr/local/bin
	chmod +x /usr/local/bin/snifferserver
	@echo '################################################################'
	@echo ''
	@echo '# Install snifferserver finished!'
	@echo ''
	@echo '################################################################'

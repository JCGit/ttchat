#!/bin/sh
SRC_DIR=./
DST_DIR=./gen

CPP_DIR=../ios/TeamTalk/Exist/ProtocolBuffers

#C++
mkdir -p $DST_DIR/cpp		
protoc --plugin=/usr/local/bin/protoc-gen-obj -I=$SRC_DIR --objc_out=$DST_DIR/cpp/ $SRC_DIR/*.proto





rm -rf CPP_DIR
#C++
cp $DST_DIR/cpp/* $CPP_DIR/

rm -rf ./gen




#JAVA
#mkdir -p $DST_DIR/java
#protoc -I=$SRC_DIR --java_out=$DST_DIR/java/ $SRC_DIR/*.proto

#PYTHON
#mkdir -p $DST_DIR/python
#protoc -I=$SRC_DIR --python_out=$DST_DIR/python/ $SRC_DIR/*.proto

httpserver: httpserver.c libuv/libuv.a http-parser/http_parser.o
	$(CC) -o httpserver \
	httpserver.c libuv/build/Release/libuv.a http-parser/http_parser.o \
	-lpthread -I libuv/include

	
libuv/libuv.a:
	cd libuv && python gyp_uv.py -f xcode
	cd libuv && xcodebuild -ARCHS="x86_64" -project uv.xcodeproj \
	    -configuration Release -target All

http-parser/http_parser.o:
	$(MAKE) -C http-parser http_parser.o

clean:
	$(MAKE) -C http-parser clean
	-$(RM) -r libuv/build
	-$(RM) -r libuv/uv.xcodeproj
	-$(RM) httpserver

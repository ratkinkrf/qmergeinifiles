qmeregeinifiles:
	g++ -g main.cpp -std=c++11 -o qmeregeinifiles


clean:
	rm qmeregeinifiles

install:
	mkdir -p $(DESTDIR)/usr/bin
	install -m 0755 qmeregeinifiles $(DESTDIR)/usr/bin/qmeregeinifiles

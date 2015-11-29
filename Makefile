CC=gcc
CXX=g++
TARGET=protector

$(TARGET): protector.cpp common.cpp slre.cpp
	$(CXX) -o $@ $^

common.cpp: common.cpp.in
	./dump_c.py $< > $@

clean:
	rm -f $(TARGET) common.cpp q1.tar.gz

q1.tar.gz: $(TARGET) protector.cpp common.cpp slre.cpp slre.h
	tar cvzf $@ *.cpp *.h
handin: q1.tar.gz

.PHONY: clean common.cpp

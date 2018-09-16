TARGET			= r
CXX 			= clang++
CPPFLAGS		= -Wno-unknown-warning-option $(shell llvm-config --cxxflags)
LDFLAGS			= $(shell llvm-config --ldflags --libs --system-libs)

$(TARGET): lex.yy.o parser.tab.o ast.o
	$(CXX) -o $@ $^ $(LDFLAGS)
lex.yy.o: lex.yy.c parser.tab.hpp ast.hpp
	$(CXX) $(CPPFLAGS) -Wno-sign-compare -c -o $@ $<
lex.yy.c: lexer.lex
	flex $<
parser.tab.o: parser.tab.cpp parser.tab.hpp ast.hpp
	$(CXX) $(CPPFLAGS) -c -o $@ $<
parser.tab.cpp parser.tab.hpp: parser.ypp
	bison -d -v $<
ast.o: ast.cpp ast.hpp
	$(CXX) $(CPPFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -f *~ *.o lex.yy.c r *.output parser.tab.* parser.cpp

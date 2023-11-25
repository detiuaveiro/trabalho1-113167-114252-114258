# make              # to compile files and create the executables
# make pgm          # to download example images to the pgm/ dir
# make setup        # to setup the test files in test/ dir
# make tests        # to run basic tests
# make clean        # to cleanup object files and executables
# make cleanobj     # to cleanup object files only

CFLAGS = -Wall -O2 -g

LDLIBS = -lm

PROGS = imageTool imageTest

TESTS = test1 test2 test3 test4 test5 test6 test7 test8 test9 

# Default rule: make all programs
all: $(PROGS)

imageTest: imageTest.o image8bit.o instrumentation.o error.o

imageTest.o: image8bit.h instrumentation.h

imageTool: imageTool.o image8bit.o instrumentation.o error.o

imageTool.o: image8bit.h instrumentation.h

# Rule to make any .o file dependent upon corresponding .h file
%.o: %.h

pgm:
	wget -O- https://sweet.ua.pt/jmr/aed/pgm.tgz | tar xzf -

.PHONY: setup
setup: test/

test/:
	wget -O- https://sweet.ua.pt/jmr/aed/test.tgz | tar xzf -
	@#mkdir -p $@
	@#curl -s -o test/aed-trab1-test.zip https://sweet.ua.pt/mario.antunes/aed/test/aed-trab1-test.zip
	@#unzip -q -o test/aed-trab1-test.zip -d test/

test1: $(PROGS) setup
	./imageTool test/original.pgm neg save neg.pgm
	cmp neg.pgm test/neg.pgm

test2: $(PROGS) setup
	./imageTool test/original.pgm thr 128 save thr.pgm
	cmp thr.pgm test/thr.pgm

test3: $(PROGS) setup
	./imageTool test/original.pgm bri .33 save bri.pgm
	cmp bri.pgm test/bri.pgm

test4: $(PROGS) setup
	./imageTool test/original.pgm rotate save rotate.pgm
	cmp rotate.pgm test/rotate.pgm

test5: $(PROGS) setup
	./imageTool test/original.pgm mirror save mirror.pgm
	cmp mirror.pgm test/mirror.pgm

test6: $(PROGS) setup
	./imageTool test/original.pgm crop 100,100,100,100 save crop.pgm
	cmp crop.pgm test/crop.pgm

test7: $(PROGS) setup
	./imageTool test/small.pgm test/original.pgm paste 100,100 save paste.pgm
	cmp paste.pgm test/paste.pgm

test8: $(PROGS) setup
	./imageTool test/small.pgm test/original.pgm blend 100,100,.33 save blend.pgm
	cmp blend.pgm test/blend.pgm

test9: $(PROGS) setup
	./imageTool test/original.pgm blur 7,7 save blur.pgm
	cmp blur.pgm test/blur.pgm

# testlocate5: $(PROGS) setup # media na grande
#     ./imageTool pgm/medium/tools_2_765x460.pgm pgm/large/ireland_03_1600x1200.pgm paste 50,222 save PasteTest.pgm
#     ./imageTool pgm/medium/tools_2_765x460.pgm PasteTest.pgm locate

# testblur1: $(PROGS) setup
#     ./imageTool pgm/medium/mandrill_512x512.pgm blur 2,9 save BlurTest.pgm
	

# Testes adicionais - Locate e Blur	
testLocate1: $(PROGS) setup
	./imageTool pgm/small/bird_256x256.pgm pgm/medium/mandrill_512x512.pgm paste 0,0 save babuino1.pgm
	./imageTool pgm/small/bird_256x256.pgm babuino1.pgm tic locate toc

# 
testLocate2: $(PROGS) setup
	./imageTool pgm/small/bird_256x256.pgm pgm/medium/mandrill_512x512.pgm paste 40,120 save babuino2.pgm
	./imageTool pgm/small/bird_256x256.pgm babuino2.pgm tic locate toc

testLocate3: $(PROGS) setup
	./imageTool pgm/small/bird_256x256.pgm pgm/medium/mandrill_512x512.pgm paste 255,255 save babuino3.pgm
	./imageTool pgm/small/bird_256x256.pgm babuino3.pgm tic locate toc

testLocate4: $(PROGS) setup
	./imageTool pgm/small/art3_222x217.pgm pgm/large/airfield-05_1600x1200.pgm paste 0,0 save aereos1.pgm
	./imageTool pgm/small/art3_222x217.pgm aereos1.pgm tic locate toc

testLocate5: $(PROGS) setup
	./imageTool pgm/small/art3_222x217.pgm pgm/large/airfield-05_1600x1200.pgm paste 700,500 save aereos2.pgm
	./imageTool pgm/small/art3_222x217.pgm aereos2.pgm tic locate toc

testLocate6: $(PROGS) setup
	./imageTool pgm/small/art3_222x217.pgm pgm/large/airfield-05_1600x1200.pgm paste 1377,982 save aereos3.pgm
	./imageTool pgm/small/art3_222x217.pgm aereos3.pgm tic locate toc

testLocate7: $(PROGS) setup
	./imageTool pgm/small/bird_256x256.pgm pgm/medium/mandrill_512x512.pgm tic locate toc

testLocate8: $(PROGS) setup
	./imageTool birdBlur1_1.pgm pgm/medium/mandrill_512x512.pgm paste 255,255 save monkeBirb.pgm
	./imageTool pgm/small/bird_256x256.pgm monkeBirb.pgm tic locate toc

testBlur1: $(PROGS) setup
	./imageTool pgm/medium/mandrill_512x512.pgm blur 15,0 save monkeBlur1.pgm

testBlur2: $(PROGS) setup
	./imageTool pgm/small/bird_256x256.pgm blur 1,1 save birdBlur1_1.pgm

.PHONY: tests
tests: $(TESTS)

# Make uses builtin rule to create .o from .c files.

cleanobj:
	rm -f *.o

clean: cleanobj
	rm -f $(PROGS)


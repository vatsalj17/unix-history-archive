CC = cc
LIBES =
OBJS = yyy0.o yyy1.o yyy2.o yyy3.o y4.o y5.o

yyyacc: $(OBJS)
	$(CC) $(OBJS) $(LIBES) -o yyyacc

install: yyyacc
	mv yyyacc ../../bin/yyyacc

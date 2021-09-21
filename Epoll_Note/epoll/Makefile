#target:dependency
#	command
SRC =$(wildcard *.c)
obj =$(patsubst %.c,%.o,$(SRC))
CC     :=  gcc
RM	   :=  rm -rf
CCFLAGS =  -o
OBJGEN  =  HelloWorld
$(OBJGEN):$(obj)
	$(CC)  $(CCFLAGS) $@ $^
.PHONY:
clean:
	$(RM) *.o *.i  *.S  $(OBJGEN)
	echo "clean up"
	
#make clean 
#make
#make search.o
#%.o:%.c %.h	
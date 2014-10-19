all:a.out

clean:
	rm -rf *.o a.out
	
a.out:asm.o context.o task.o thread.o main.o channel.o fd.o net.o test.o
	gcc $^ -o a.out
	
asm.o:asm.S
	gcc -c $< -o $@
	
channel.o:channel.c
	gcc -c $< -o $@

context.o:context.c
	gcc -c $< -o $@
	
task.o:task.c impl.h task.h
	gcc -c -g $< -o $@
	
main.o:main.c impl.h
	gcc -c -g $< -o $@

thread.o:thread.c impl.h
	gcc -c -g $< -o $@
	
fd.o:fd.c
	gcc -c $< -o $@
	
net.o:net.c
	gcc -c $< -o $@
	
test.o: test.c task.h
	gcc -c -g $< -o $@
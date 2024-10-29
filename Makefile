redirect_output: redirect_output.c
	gcc -o redirect_output redirect_output.c

clean:
	rm -f redirect_output
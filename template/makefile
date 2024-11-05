OPTIONS = -framework GLUT -framework OpenGL
DEFINES = -D GL_SILENCE_DEPRECATION

template: template.c initShader.o tempLib.o
	gcc -o template template.c initShader.o tempLib.o $(OPTIONS) $(DEFINES)

initShader.o: initShader.c initShader.h
	gcc -c initShader.c $(DEFINES)

tempLib.o: tempLib.c tempLib.h
	gcc -c tempLib.c $(DEFINES)

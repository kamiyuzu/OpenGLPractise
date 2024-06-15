# OpenGLPractise
OpenGL practise for an university subject

## How to compile the proyect
First to compile the project just cd into the build folder an execute the following command:

```sh
cmake .. -DOpenGL_GL_PREFERENCE=GLVND && make -j4
```

Afterwards just run the OpenGL generated binary you want to visualise:

```sh
./bin/gpo_04_entrega_ejer3
```

## Cel-shading
GIF illustrating the cel-shading implemented:
![Cel-shading](recordings/cel-shading.gif)
## Stippling
GIF illustrating the stippling implemented changing the factor:
![StipplingFactor](recordings/stippling-factor-change.gif)
GIF illustrating the stippling implemented changing the pattern:
![StipplingPattern](recordings/stippling-pattern-change.gif)
## Ink texture NPR
GIF illustrating the ink texture implemented changing the factor:
![INK_Texture](recordings/Ink_Texture.gif)
## Ink Wash
GIF illustrating the ink wash texture implemented changing the factor:
![INK_Texture](recordings/inkwash.gif)

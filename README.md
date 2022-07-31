# pumlconv
A small C code to convert pseudo code ( in C like syntax ) to PlantUML activity diagram (beta) language.

# Dependencies
. plantuml: its best to install it with `sudo apt install plantuml`

# installation:
``` sh
make
sudo make install
```

# usage
Simply run
``` sh
ac.sh [input file name] [output image name]
```

<b>Or you can do it manually:</b>

In order to convert your code to plantUML code simply run:
``` sh
conv -f [input file name] -o [output file name]
```
Then you can pass it to plantUML directly:
``` sh
plantuml [plantuml code file name]
```
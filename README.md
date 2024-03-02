# LearnWGPU

## Windows

On windows, first run the init.bat file:

```
.\init.bat
```

To generate build files with cmake (generate for x86 win32):

```
cmake -B build -T host=x86 -A win32
```

## Linux

On Linux, first run the init.sh file:

```
source init.sh
```

To generate build files using cmake:

```
cmake -B build
```

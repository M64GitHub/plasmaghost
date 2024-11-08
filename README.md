# PLASMAGHOST
I am sharing this as test- and example-code for [libtsprites](https://github.com/M64GitHub/libtsprites). It's a funny program to test your terminal speed ;)


https://github.com/user-attachments/assets/303a5164-0719-4e7d-8466-a5b553ef2583


## BUILD

```bash
cd yourbuilddir
git clone https://github.com/M64GitHub/libtsprites.git
git clone https://github.com/M64GitHub/plasmaghost.git
cd plasmaghost
make
```
Please note, this is test code, I use this setup while working on libtsprites and test code simultaneously. You can skip building / "installing" a shared lib this way.

## RUN

```bash
./plasmaghost
```
Please mind, plasmaghost loads it's assets (png files) from the 'resources/' subfolder. This is on purpose, as this is just test code and to keep experimenting flexible. Later those files can be converted to header files, zipped, etc (LibTSprites does not yet support such features).

## CONTROL

- Use cursor keys / vi keys to move up / right / down / left
- ESC / q to quit
- CTRL-C to quit


cd fd
./genfd.sh
cd ..

mkdir package
mkdir package/samples

mkdir package/samples/Asm
cp samples/Asm/AsmSample.s package/samples/Asm
cp samples/Asm/Fog.i package/samples/Asm
cp samples/Asm/Makefile.AsmOnly package/samples/Asm/Makefile

mkdir package/samples/ImmediateDraw
cp samples/ImmediateDraw/immediate.c package/samples/ImmediateDraw
cp samples/ImmediateDraw/Makefile package/samples/ImmediateDraw

mkdir package/samples/Lighting
cp samples/Lighting/lighting.c package/samples/Lighting
cp samples/Lighting/Makefile package/samples/Lighting

mkdir package/samples/QuadCube
cp samples/QuadCube/quadcube.c package/samples/QuadCube
cp samples/QuadCube/Makefile package/samples/QuadCube

mkdir package/samples/UserProvided
cp samples/UserProvided/userdraw.c package/samples/UserProvided
cp samples/UserProvided/Makefile package/samples/UserProvided

mkdir package/samples/ZBuffer
cp samples/ZBuffer/font8x8.h package/samples/ZBuffer
cp samples/ZBuffer/zbuffer.c package/samples/ZBuffer
cp samples/ZBuffer/Makefile package/samples/ZBuffer

mkdir package/samples/Windowed
cp samples/Windowed/windowed.c package/samples/Windowed
cp samples/Windowed/Makefile package/samples/Windowed

mkdir package/samples/Sprites
cp samples/Sprites/sprites.c package/samples/Sprites
cp samples/Sprites/Makefile package/samples/Sprites
cp samples/Sprites/fire0.dds package/samples/Sprites
cp samples/Sprites/fire1.dds package/samples/Sprites
cp samples/Sprites/fire2.dds package/samples/Sprites
cp samples/Sprites/fire3.dds package/samples/Sprites

mkdir package/samples/E
cp samples/E/* package/samples/E

mkdir package/samples/GLWrapper
cp samples/GLWrapper/* package/samples/GLWrapper

mkdir package/include
cp -r fd/* package/include
cp include/maggie* package/include

make clean
mkdir package/libs
make
make strip
cp maggie.library package/libs
make clean

cp Readme.txt package/

mkdir package/bin

cd package/samples

cd Asm
make
make strip
cp AsmSample ../../bin
make clean
cd ..

cd ImmediateDraw
make
make strip
cp ImmediateDraw ../../bin
make clean
cd ..

cd Lighting
make
make strip
cp Lighting ../../bin
make clean
cd ..

cd QuadCube
make
make strip
cp QuadCube ../../bin
make clean
cd ..

cd UserProvided
make
make strip
cp UserDraw ../../bin
make clean
cd ..

cd ZBuffer
make
make strip
cp ZBuffer ../../bin
make clean
cd ..

cd Windowed
make
make strip
cp Windowed ../../bin
make clean
cd ..

cd ../..

cp samples/ZBuffer/TestTexture.dds package/bin/

cd ..
